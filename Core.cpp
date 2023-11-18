#include "pch.h"
#include "Core.h"
#include "PathMgr.h"
#include "ResMgr.h"
#include "SceneMgr.h"
#include "TimeMgr.h"
#include "KeyMgr.h"
#include "Texture.h"
#include "Scene.h"
#include "ThreadMgr.h"
#include "EventMgr.h"
#include "ImGuiMgr.h"
#include "UIMgr.h"
#include "AssimpMgr.h"
#include "ParticleMgr.h"
#include "CollisionMgr.h"
#include "RayCaster.h"
#include "SoundMgr.h"

Core::Core()
{
	m_vecDrawCall.reserve(1000);
}

Core::~Core()
{
	glDeleteBuffers(1, &m_ubo);

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(m_hglrc);
	ReleaseDC(m_hWnd, m_hDC);

	glfwDestroyWindow(m_pWinInfo);
	glfwTerminate();
}

void Core::SetUBO()
{
	glGenBuffers(1, &m_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UBOData), nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

std::tuple<int, int, int, int> Core::AdjustWinSize(GLuint width_, GLuint height_, HWND hwnd)const noexcept
{
	const HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
	GetMonitorInfo(hMonitor, &monitorInfo);

	const int monitorWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	const int monitorHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

	const UINT dpi = GetDpiForWindow(hwnd);
	const float scaleFactor = dpi / 96.0f;

	const int adjustedWidth = static_cast<int>(width_ * scaleFactor);
	const int adjustedHeight = static_cast<int>(height_ * scaleFactor);

	const int posX = (monitorWidth - adjustedWidth) / 2;
	const int posY = (monitorHeight - adjustedHeight) / 2;

	return std::make_tuple(adjustedWidth, adjustedHeight, posX, posY);
}

std::tuple<int, int, int, int> Core::AdjustWinSize(HWND hwnd) const noexcept
{
	const HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
	GetMonitorInfo(hMonitor, &monitorInfo);

	const int monitorWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	const int monitorHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

	const UINT dpi = GetDpiForWindow(hwnd);
	const float scaleFactor = dpi / 96.0f;

	int adjustedWidth = static_cast<int>(monitorWidth * scaleFactor);
	int adjustedHeight = adjustedWidth / 2;


	if (adjustedHeight > monitorHeight * 0.8f)
	{
		adjustedHeight = static_cast<int>(monitorHeight * 0.8f);
		adjustedWidth = adjustedHeight * 2;
	}

	const int posX = (monitorWidth - adjustedWidth) / 2;
	const int posY = (monitorHeight - adjustedHeight) / 2;

	return std::make_tuple(adjustedWidth, adjustedHeight, posX, posY);
}

void Core::Init(const GLuint _winWidth, const GLuint _winHeight)
{
	glfwInit();
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	
	m_pWinInfo = glfwCreateWindow(1, 1, "MyOpenGL", nullptr, nullptr);

	m_hWnd = glfwGetWin32Window(m_pWinInfo);
	m_hDC = GetDC(m_hWnd);
	m_hglrc = wglCreateContext(m_hDC);

	int posX, posY;

	std::tie(m_winWidth, m_winHeight, posX, posY) = 0 == _winWidth || 0 == _winHeight ? AdjustWinSize(m_hWnd) : AdjustWinSize(_winWidth, _winHeight, m_hWnd);

	glfwSetWindowPos(m_pWinInfo, posX, posY);
	glfwSetWindowSize(m_pWinInfo, m_winWidth, m_winHeight);

	wglMakeCurrent(m_hDC, m_hglrc);

	glewInit();

	const GLubyte* version = glGetString(GL_VERSION);
	std::cout << "OpenGL Version: " << version << std::endl;

	glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a);

	glEnable(GL_DEPTH_TEST);
	glDepthRange(0.0F, 1.0F);
	glClearDepthf(1.0F);

	//glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CCW);
	//glCullFace(GL_BACK);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glfwSetWindowRefreshCallback(m_pWinInfo, [](GLFWwindow* _winInfo) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Mgr(SceneMgr)->Render();

		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		Mgr(ImGuiMgr)->Render();

		glfwSwapBuffers(_winInfo);
		});

	glfwSetFramebufferSizeCallback(m_pWinInfo, [](GLFWwindow* _window, int width, int height) {
		Mgr(Core)->SetWinWidth(width);
		Mgr(Core)->SetWinHeight(height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Mgr(SceneMgr)->Update();
		Mgr(SceneMgr)->Render();

		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		Mgr(ImGuiMgr)->Update();
		Mgr(ImGuiMgr)->Render();

		glfwSwapBuffers(_window);
		});

	glfwSetKeyCallback(m_pWinInfo, [](GLFWwindow* _window, int key, int scancode, int action, int mods) {
		ImGui_ImplGlfw_KeyCallback(_window, key, scancode, action, mods);

		});

	glfwSetCharCallback(m_pWinInfo, [](GLFWwindow* _window, unsigned int ch) {
		ImGui_ImplGlfw_CharCallback(_window, ch);
		});

	glfwSetCursorPosCallback(m_pWinInfo, [](GLFWwindow* _window, double x, double y) {
		ImGui_ImplGlfw_CursorPosCallback(_window, x, y);
		});

	glfwSetMouseButtonCallback(m_pWinInfo, [](GLFWwindow* _window, int button, int action, int modifier) {
		ImGui_ImplGlfw_MouseButtonCallback(_window, button, action, modifier);
		});

	glfwSetScrollCallback(m_pWinInfo, [](GLFWwindow* _window, double xoffset, double yoffset) {
		ImGui_ImplGlfw_ScrollCallback(_window, xoffset, yoffset);
		});

	glViewport(0, 0, m_winWidth, m_winHeight);
	glfwSwapInterval(1);

	SetUBO();

	Mgr(ThreadMgr)->Init();
	Mgr(PathMgr)->Init();
	Mgr(ResMgr)->Init();
	Mgr(SceneMgr)->Init();
	Mgr(KeyMgr)->Init();
	Mgr(ImGuiMgr)->Init();
	Mgr(UIMgr)->Init();
	Mgr(EventMgr)->Init();
	Mgr(ParticleMgr)->Init();
	Mgr(AssimpMgr)->Init();
	Mgr(CollisionMgr)->Init();
	Mgr(RayCaster)->Init();
	Mgr(SoundMgr)->Init();

	Mgr(TimeMgr)->Init();
}

void Core::GameLoop()
{
	Mgr(SceneMgr)->GetCurScene()->Awake();
	Mgr(SceneMgr)->GetCurScene()->Start();
	Mgr(UIMgr)->Start();
	Mgr(ThreadMgr)->ExecuteMainThreadTask();
	
	while (!glfwWindowShouldClose(m_pWinInfo))
	{
		glfwPollEvents();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		Mgr(KeyMgr)->Update();
		Mgr(TimeMgr)->Update();
		Mgr(SceneMgr)->Update();

		Mgr(SceneMgr)->PreFinalUpdate();

		Mgr(CollisionMgr)->CollisionUpdate();

		Mgr(SceneMgr)->FinalUpdate();

		Mgr(RayCaster)->Update();

		Mgr(SceneMgr)->Render();
		
		Mgr(ParticleMgr)->Update();

		Mgr(CollisionMgr)->RenderCollisionBox();

		//DrawCall();

		Mgr(UIMgr)->Update();
		Mgr(UIMgr)->Render();

		//DrawCall();
		Mgr(EventMgr)->Update();
		Mgr(SoundMgr)->Update();

		Mgr(ImGuiMgr)->Update();
		Mgr(ImGuiMgr)->Render();

		glfwSwapBuffers(m_pWinInfo);
	}
}

void Core::Quit() const noexcept
{
	glfwSetWindowShouldClose(m_pWinInfo, GL_TRUE);
}
