#pragma once
class CIQtestmachineTool
{
public:
    static void Dump(BYTE* pData, size_t nSize)
    {
        std::string strOut;
        for (size_t i = 0; i < nSize; i++)
        {
            char buf[8] = "";
            if (i > 0 && (i % 16) == 0)
                strOut += "\n";
            snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
            strOut += buf;
        }
        strOut += "\n";
        OutputDebugStringA(strOut.c_str());
    }


    static bool IsAdmin()//管理员权限检测
    {
        HANDLE hToken = nullptr;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        {
            ShowError();
            return false;
        }
        TOKEN_ELEVATION eve;
        DWORD len = 0;
        if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == false)
        {
            ShowError();
            return false;
        }
        CloseHandle(hToken);
        if (len == sizeof(eve))
            return eve.TokenIsElevated;
        return false;
    }

    static bool Init()//初始化MFC函数
    {
        HMODULE hModule = ::GetModuleHandle(nullptr);
        if (hModule == nullptr)
        {
            wprintf(L"错误: GetModuleHandle 失败\n");
            return false;
        }
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            return false;
        }
        return true;
    }

    static void ShowError()//错误处理
    {
        LPWSTR lpMessageBuf = nullptr;
        //strerror(error) 标准C语言库
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&lpMessageBuf, 0, nullptr);
        OutputDebugString(lpMessageBuf);
        LocalFree(lpMessageBuf);
    }

//服务端开机启动函数: 第一种方法
//开机启动的时候, 程序能否启动(即是已经在注册表里注册)与启动用户的权限有关, 例如程序的启动权限是管理员级别, 而启动用户的权限是普通用户, 则不会启动程序
//开机启动对环境变量有影响, 如果依赖dll(动态库), 则可能启动失败
//若要使用动态库, 复制这些dll到文件夹system32(!!!多是64位程序)下面sysWOW64(!!!多是32位程序)下面
    static bool WriteRegisterTable(const CString& strPath)//第一种开机启动方式, 通过注册表, 程序在开机伴随用户登入而启动
    {
        CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");

        ////软链接方式
        //char sSys[MAX_PATH] = "";
        //char sPath[MAX_PATH] = "";
        //std::string strExe = "\\RemoteCtrl.exe ";
        //GetCurrentDirectoryA(MAX_PATH, sPath);
        //GetSystemDirectoryA(sSys, sizeof(sSys));
        //std::string strCmd = "mklink " + std::string(sSys) + strExe + std::string(sPath) + strExe;
        //system(strCmd.c_str());

        //拷贝整个.exe方式
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(nullptr, sPath, MAX_PATH);
        bool ret = CopyFile(sPath, strPath, FALSE);
        if (ret == false)
        {
            MessageBox(nullptr, _T("复制文件失败, 是否权限不足?\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        HKEY hKey = nullptr;
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_WRITE | KEY_WOW64_64KEY, &hKey);//打开注册表下的指定路径
        if (ret != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("设置自动开机启动失败! 是否权限不足?\r\n程序启动失败!"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        //CString strPath = CString(_T("%SystemRoot%\\SysWOW64\\RemoteCtrl.exe"));
        ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));//添加到注册表的指定路径下
        if (ret != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("设置自动开机启动失败! 是否权限不足?\r\n程序启动失败!"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        RegCloseKey(hKey);
        return true;
    }

    static bool WriteStartupDir(const CString& strPath)//第二种开机启动方式, 写入启动文件夹, 程序在用户登入后启动
    {
       /* CString strCmd = GetCommandLine();
        strCmd.Replace(_T("\""), _T(""));
        bool ret = CopyFile(strCmd, strPath, FALSE);*/
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(nullptr, sPath, MAX_PATH);
        bool ret = CopyFile(sPath, strPath, FALSE);
        if (ret == false)
        {
            MessageBox(nullptr, _T("复制文件失败, 是否权限不足?\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        return true;
    }


    static bool RunAsAdmin()//使用管理员权限创建进程 
    {
        //需要在本地策略组: 启用管理员账户, 禁用空密码只能登录本地控制台

        ////获取管理员权限
        //HANDLE hToken = nullptr;
        //bool ret = LogonUser(L"Administrator", NULL, NULL, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken);
        //if (!ret)
        //{
        //    MessageBox(nullptr, _T("管理员用户登入失败"), _T("程序错误"), 0);
        //    ShowError();
        //    exit(0);
        //}
        //OutputDebugString(L"Logon Administrator success!\r\n");
        //CloseHandle(hToken);

        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(nullptr, sPath, MAX_PATH);
        bool ret = CreateProcessWithLogonW(_T("Administrator"), nullptr, nullptr, LOGON_WITH_PROFILE, nullptr, (LPWSTR)(LPCWSTR)sPath, CREATE_UNICODE_ENVIRONMENT, nullptr, nullptr, &si, &pi);

        if (!ret)
        {
            MessageBox(nullptr, sPath, _T("创建进程失败"), 0);
            ShowError();
            return false;
        }
        //MessageBox(nullptr, sPath, _T("创建进程成功"), 0);
        WaitForSingleObject(pi.hProcess, INFINITE);//等待创建的子进程结束
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
};



