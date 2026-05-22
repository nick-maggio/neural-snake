#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <vector>

#include "game_state.h"
#include "neural_agent.h"
#include "population.h"
#include "showcase.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;

static const int WINDOW_W = 1280;
static const int WINDOW_H = 720;

static const double STEP_MS        = 25.0;
static const double POST_PHASE1_MS = 250.0;

static const int POP_SIZE    = 150;
static const int GENERATIONS = 700;
static const int BOARD_W     = 10;
static const int BOARD_H     = 10;
// Only 4 is rendered in the DX11 window; other values still train but skip the visual.
static const int SHOWCASE_N  = 4;

static const int LAYOUT_PADDING = 24;
// Fit two rows vertically with three gutters; use the same value horizontally to keep boards square.
static const int QUADRANT_SIZE  = (WINDOW_H - 3 * LAYOUT_PADDING) / 2;
static const int CELL_SIZE      = QUADRANT_SIZE / BOARD_W;
static const int BOARD_PX       = CELL_SIZE * BOARD_W;
static const int GRID_W         = BOARD_PX * 2 + LAYOUT_PADDING;
static const int GRID_ORIGIN_X  = (WINDOW_W - GRID_W) / 2;
static const int GRID_ORIGIN_Y  = LAYOUT_PADDING;

struct Color { float r, g, b, a; };
static const Color COLOR_BG          = {0.05f, 0.05f, 0.08f, 1.0f};
static const Color COLOR_BOARD_BG    = {0.12f, 0.12f, 0.15f, 1.0f};
static const Color COLOR_BORDER      = {0.35f, 0.35f, 0.40f, 1.0f};
static const Color COLOR_BORDER_DEAD = {0.45f, 0.20f, 0.20f, 1.0f};
static const Color COLOR_SNAKE_BODY  = {0.20f, 0.70f, 0.30f, 1.0f};
static const Color COLOR_SNAKE_HEAD  = {0.50f, 1.00f, 0.55f, 1.0f};
static const Color COLOR_SNAKE_DEAD  = {0.40f, 0.40f, 0.40f, 1.0f};
static const Color COLOR_FOOD        = {0.90f, 0.25f, 0.25f, 1.0f};

static ComPtr<ID3D11Device>           g_device;
static ComPtr<ID3D11DeviceContext>    g_context;
static ComPtr<IDXGISwapChain>         g_swapChain;
static ComPtr<ID3D11RenderTargetView> g_rtv;
static ComPtr<ID3D11VertexShader>     g_vs;
static ComPtr<ID3D11PixelShader>      g_ps;
static ComPtr<ID3D11InputLayout>      g_inputLayout;
static ComPtr<ID3D11Buffer>           g_quadVB;
static ComPtr<ID3D11Buffer>           g_quadIB;
static ComPtr<ID3D11Buffer>           g_instanceVB;
static ComPtr<ID3D11Buffer>           g_constantBuffer;

static const UINT MAX_INSTANCES = 100000;

struct Vertex { float x, y; };

struct InstanceData {
  float posX, posY;
  float sizeX, sizeY;
  float r, g, b, a;
};

struct FrameData {
  float viewportW, viewportH;
  float pad0, pad1;
};

static std::vector<InstanceData> g_rects;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  switch (msg) {
  case WM_DESTROY: PostQuitMessage(0); return 0;
  }
  return DefWindowProc(hwnd, msg, wp, lp);
}

HWND CreateAppWindow(HINSTANCE hInstance, int width, int height) {
  WNDCLASS wc{};
  wc.lpfnWndProc   = WndProc;
  wc.hInstance     = hInstance;
  wc.lpszClassName = L"SnakeRendererWindow";
  wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
  RegisterClass(&wc);
  RECT r{ 0, 0, width, height };
  AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
  return CreateWindow(wc.lpszClassName, L"Neural Snake - DX11",
    WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
    r.right - r.left, r.bottom - r.top, nullptr, nullptr, hInstance, nullptr);
}

bool InitD3D(HWND hwnd, int width, int height) {
  DXGI_SWAP_CHAIN_DESC scd{};
  scd.BufferCount        = 1;
  scd.BufferDesc.Width   = width;
  scd.BufferDesc.Height  = height;
  scd.BufferDesc.Format  = DXGI_FORMAT_R8G8B8A8_UNORM;
  scd.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  scd.OutputWindow       = hwnd;
  scd.SampleDesc.Count   = 1;
  scd.Windowed           = TRUE;

  HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
    nullptr, 0, D3D11_SDK_VERSION, &scd,
    g_swapChain.GetAddressOf(), g_device.GetAddressOf(), nullptr, g_context.GetAddressOf());
  if (FAILED(hr)) return false;

  ComPtr<ID3D11Texture2D> backBuffer;
  hr = g_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
  if (FAILED(hr)) return false;
  hr = g_device->CreateRenderTargetView(backBuffer.Get(), nullptr, g_rtv.GetAddressOf());
  if (FAILED(hr)) return false;
  g_context->OMSetRenderTargets(1, g_rtv.GetAddressOf(), nullptr);

  D3D11_VIEWPORT vp{};
  vp.Width = (FLOAT)width; vp.Height = (FLOAT)height; vp.MaxDepth = 1.0f;
  g_context->RSSetViewports(1, &vp);
  return true;
}

bool CompileShader(const wchar_t* file, const char* entry, const char* profile, ComPtr<ID3DBlob>& blob) {
  ComPtr<ID3DBlob> errors;
  HRESULT hr = D3DCompileFromFile(file, nullptr, nullptr, entry, profile, 0, 0,
    blob.GetAddressOf(), errors.GetAddressOf());
  if (FAILED(hr)) {
    if (errors) MessageBoxA(nullptr, (char*)errors->GetBufferPointer(), "Shader compile error", MB_OK);
    return false;
  }
  return true;
}

bool InitPipeline() {
  ComPtr<ID3DBlob> vsBlob, psBlob;
  if (!CompileShader(L"shaders/shaders.hlsl", "VSMain", "vs_5_0", vsBlob)) return false;
  if (!CompileShader(L"shaders/shaders.hlsl", "PSMain", "ps_5_0", psBlob)) return false;

  HRESULT hr;
  hr = g_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, g_vs.GetAddressOf());
  if (FAILED(hr)) return false;
  hr = g_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, g_ps.GetAddressOf());
  if (FAILED(hr)) return false;

  D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION",       0, DXGI_FORMAT_R32G32_FLOAT,       0, 0,
      D3D11_INPUT_PER_VERTEX_DATA,   0 },
    { "INSTANCE_POS",   0, DXGI_FORMAT_R32G32_FLOAT,       1, 0,
      D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "INSTANCE_SIZE",  0, DXGI_FORMAT_R32G32_FLOAT,       1, D3D11_APPEND_ALIGNED_ELEMENT,
      D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "INSTANCE_COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT,
      D3D11_INPUT_PER_INSTANCE_DATA, 1 },
  };
  hr = g_device->CreateInputLayout(layout, _countof(layout),
    vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), g_inputLayout.GetAddressOf());
  if (FAILED(hr)) return false;

  Vertex verts[] = { {0,0}, {1,0}, {1,1}, {0,1} };
  D3D11_BUFFER_DESC vbd{}; vbd.Usage = D3D11_USAGE_IMMUTABLE; vbd.ByteWidth = sizeof(verts); vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  D3D11_SUBRESOURCE_DATA vinit{}; vinit.pSysMem = verts;
  hr = g_device->CreateBuffer(&vbd, &vinit, g_quadVB.GetAddressOf()); if (FAILED(hr)) return false;

  uint16_t indices[] = { 0, 2, 3, 0, 1, 2 };
  D3D11_BUFFER_DESC ibd{}; ibd.Usage = D3D11_USAGE_IMMUTABLE; ibd.ByteWidth = sizeof(indices); ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  D3D11_SUBRESOURCE_DATA iinit{}; iinit.pSysMem = indices;
  hr = g_device->CreateBuffer(&ibd, &iinit, g_quadIB.GetAddressOf()); if (FAILED(hr)) return false;

  D3D11_BUFFER_DESC instbd{};
  instbd.Usage          = D3D11_USAGE_DYNAMIC;
  instbd.ByteWidth      = sizeof(InstanceData) * MAX_INSTANCES;
  instbd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
  instbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  hr = g_device->CreateBuffer(&instbd, nullptr, g_instanceVB.GetAddressOf());
  if (FAILED(hr)) return false;

  D3D11_BUFFER_DESC cbd{};
  cbd.Usage          = D3D11_USAGE_DYNAMIC;
  cbd.ByteWidth      = sizeof(FrameData);
  cbd.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
  cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  hr = g_device->CreateBuffer(&cbd, nullptr, g_constantBuffer.GetAddressOf());
  if (FAILED(hr)) return false;

  return true;
}

static void pushRect(float x, float y, float w, float h, Color c) {
  InstanceData r;
  r.posX = x; r.posY = y;
  r.sizeX = w; r.sizeY = h;
  r.r = c.r; r.g = c.g; r.b = c.b; r.a = c.a;
  g_rects.push_back(r);
}

static void drawGame(const GameState& game, int originX, int originY, int cellSize) {
  int boardPxW = game.getWidth()  * cellSize;
  int boardPxH = game.getHeight() * cellSize;

  pushRect((float)originX, (float)originY,
           (float)boardPxW, (float)boardPxH, COLOR_BOARD_BG);

  Color borderColor = game.isAlive() ? COLOR_BORDER : COLOR_BORDER_DEAD;
  const float BT = 2.0f;
  pushRect((float)originX - BT,        (float)originY - BT,
           (float)boardPxW + 2*BT, BT, borderColor);
  pushRect((float)originX - BT,        (float)(originY + boardPxH),
           (float)boardPxW + 2*BT, BT, borderColor);
  pushRect((float)originX - BT,        (float)originY,
           BT, (float)boardPxH,         borderColor);
  pushRect((float)(originX + boardPxW),(float)originY,
           BT, (float)boardPxH,         borderColor);

  Color bodyColor = game.isAlive() ? COLOR_SNAKE_BODY : COLOR_SNAKE_DEAD;
  Color headColor = game.isAlive() ? COLOR_SNAKE_HEAD : COLOR_SNAKE_DEAD;

  const auto& snake = game.getSnake();
  bool first = true;
  for (const auto& seg : snake) {
    if (first) { first = false; continue; }
    float px = (float)(originX + seg.x * cellSize);
    float py = (float)(originY + seg.y * cellSize);
    pushRect(px, py, (float)cellSize, (float)cellSize, bodyColor);
  }

  Point food = game.getFood();
  pushRect((float)(originX + food.x * cellSize),
           (float)(originY + food.y * cellSize),
           (float)cellSize, (float)cellSize, COLOR_FOOD);

  Point head = game.getHead();
  pushRect((float)(originX + head.x * cellSize),
           (float)(originY + head.y * cellSize),
           (float)cellSize, (float)cellSize, headColor);
}

static void drawShowcase(const Showcase& sc) {
  for (int i = 0; i < sc.size() && i < 4; ++i) {
    int col     = i % 2;
    int row     = i / 2;
    int originX = GRID_ORIGIN_X + col * (BOARD_PX + LAYOUT_PADDING);
    int originY = GRID_ORIGIN_Y + row * (BOARD_PX + LAYOUT_PADDING);
    drawGame(sc.game(i), originX, originY, CELL_SIZE);
  }
}

void RenderFrame() {
  g_context->ClearRenderTargetView(g_rtv.Get(), (const float*)&COLOR_BG);

  D3D11_MAPPED_SUBRESOURCE mapped;
  g_context->Map(g_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
  FrameData* fd = (FrameData*)mapped.pData;
  fd->viewportW = (float)WINDOW_W;
  fd->viewportH = (float)WINDOW_H;
  g_context->Unmap(g_constantBuffer.Get(), 0);

  UINT count = (UINT)g_rects.size();
  if (count > MAX_INSTANCES) count = MAX_INSTANCES;
  if (count > 0) {
    g_context->Map(g_instanceVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, g_rects.data(), sizeof(InstanceData) * count);
    g_context->Unmap(g_instanceVB.Get(), 0);
  }

  ID3D11Buffer* vbs[]    = { g_quadVB.Get(), g_instanceVB.Get() };
  UINT          strides[] = { sizeof(Vertex), sizeof(InstanceData) };
  UINT          offsets[] = { 0, 0 };
  g_context->IASetVertexBuffers(0, 2, vbs, strides, offsets);

  g_context->IASetIndexBuffer(g_quadIB.Get(), DXGI_FORMAT_R16_UINT, 0);
  g_context->IASetInputLayout(g_inputLayout.Get());
  g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  g_context->VSSetShader(g_vs.Get(), nullptr, 0);
  g_context->VSSetConstantBuffers(0, 1, g_constantBuffer.GetAddressOf());
  g_context->PSSetShader(g_ps.Get(), nullptr, 0);

  if (count > 0) {
    g_context->DrawIndexedInstanced(6, count, 0, 0, 0);
  }

  g_swapChain->Present(1, 0);
}

static bool pumpMessages() {
  MSG msg;
  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) return false;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return true;
}

struct TopPicks {
  std::vector<std::vector<double>> genomes;
  std::vector<uint32_t>            seeds;
  std::vector<double>              fitnesses;
};

static TopPicks pickTop(const Population& pop, int n) {
  TopPicks t;
  std::vector<int> idx = pop.topNIndices(n);
  for (int i : idx) {
    t.genomes.push_back(pop.genome(i));
    t.seeds.push_back(pop.gameSeed(i));
    t.fitnesses.push_back(pop.fitness(i));
  }
  return t;
}

// Returns false if the user closed the window before the showcase finished.
static bool runShowcase(Showcase& sc, double lingerMs) {
  using clock = std::chrono::high_resolution_clock;
  auto lastTime = clock::now();
  double accumulatorMs = 0.0;
  double lingerLeftMs  = -1.0;

  while (true) {
    if (!pumpMessages()) return false;

    auto now = clock::now();
    double deltaMs = std::chrono::duration<double, std::milli>(now - lastTime).count();
    lastTime = now;

    if (!sc.allDead()) {
      accumulatorMs += deltaMs;
      while (accumulatorMs >= STEP_MS) {
        sc.step();
        accumulatorMs -= STEP_MS;
      }
    } else {
      if (lingerLeftMs < 0.0) lingerLeftMs = lingerMs;
      else                    lingerLeftMs -= deltaMs;
      if (lingerLeftMs <= 0.0) {
        g_rects.clear();
        drawShowcase(sc);
        RenderFrame();
        return true;
      }
    }

    g_rects.clear();
    drawShowcase(sc);
    RenderFrame();
  }
}

static void runShowcaseUntilClosed(Showcase& sc) {
  using clock = std::chrono::high_resolution_clock;
  auto lastTime = clock::now();
  double accumulatorMs = 0.0;

  while (true) {
    if (!pumpMessages()) return;

    auto now = clock::now();
    double deltaMs = std::chrono::duration<double, std::milli>(now - lastTime).count();
    lastTime = now;

    if (!sc.allDead()) {
      accumulatorMs += deltaMs;
      while (accumulatorMs >= STEP_MS) {
        sc.step();
        accumulatorMs -= STEP_MS;
      }
    }

    g_rects.clear();
    drawShowcase(sc);
    RenderFrame();
  }
}

int main() {
  HINSTANCE hInstance = GetModuleHandle(nullptr);
  HWND hwnd = CreateAppWindow(hInstance, WINDOW_W, WINDOW_H);
  if (!hwnd) return 1;

  if (!InitD3D(hwnd, WINDOW_W, WINDOW_H)) {
    MessageBox(hwnd, L"DX11 init failed.", L"Error", MB_OK); return 1;
  }
  if (!InitPipeline()) {
    MessageBox(hwnd, L"Pipeline init failed.", L"Error", MB_OK); return 1;
  }

  uint32_t seed = (uint32_t)std::chrono::high_resolution_clock::now()
                            .time_since_epoch().count();
  std::cout << "Seed: " << seed << "\n";
  std::cout << std::fixed << std::setprecision(2);

  Population pop(POP_SIZE, BOARD_W, BOARD_H, 11, 16, 3, seed);

  GenerationStats gen1stats = pop.runGeneration();
  std::cout << "Gen   1 | bestFit=" << gen1stats.bestFitness
            << " bestScore=" << gen1stats.bestScore << "\n";

  // Pull picks before the next runGeneration() — genomes/seeds are overwritten each generation.
  TopPicks picks1 = pickTop(pop, SHOWCASE_N);
  std::cout << "Showcasing gen 1 top " << SHOWCASE_N << "...\n";
  Showcase sc;
  sc.reset(BOARD_W, BOARD_H, picks1.genomes, picks1.seeds);
  if (!runShowcase(sc, POST_PHASE1_MS)) return 0;

  std::cout << "Training generations 2.." << GENERATIONS << "...\n";
  auto trainStart = std::chrono::high_resolution_clock::now();
  GenerationStats lastStats{};
  for (int gen = 2; gen <= GENERATIONS; ++gen) {
    lastStats = pop.runGeneration();

    if (!pumpMessages()) return 0;
    g_rects.clear();
    drawShowcase(sc);
    RenderFrame();

    if (gen % 10 == 0 || gen == GENERATIONS) {
      std::cout << "Gen " << std::setw(3) << gen
                << " | bestFit=" << lastStats.bestFitness
                << " bestScore=" << lastStats.bestScore
                << " avgScore=" << lastStats.avgScore << "\n";
    }
  }
  auto trainEnd = std::chrono::high_resolution_clock::now();
  double trainSec = std::chrono::duration<double>(trainEnd - trainStart).count();
  std::cout << "Training took " << trainSec << " seconds ("
            << (trainSec / (GENERATIONS - 1)) * 1000.0 << " ms/gen avg)\n";

  TopPicks picksFinal = pickTop(pop, SHOWCASE_N);
  std::cout << "Showcasing final gen top " << SHOWCASE_N << ":\n";
  for (int i = 0; i < (int)picksFinal.fitnesses.size(); ++i) {
    std::cout << "  rank " << i << " fit=" << picksFinal.fitnesses[i] << "\n";
  }
  sc.reset(BOARD_W, BOARD_H, picksFinal.genomes, picksFinal.seeds);
  runShowcaseUntilClosed(sc);

  return 0;
}
