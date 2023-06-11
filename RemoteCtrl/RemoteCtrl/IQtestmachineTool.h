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


    static bool IsAdmin()//����ԱȨ�޼��
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

    static bool Init()//��ʼ��MFC����
    {
        HMODULE hModule = ::GetModuleHandle(nullptr);
        if (hModule == nullptr)
        {
            wprintf(L"����: GetModuleHandle ʧ��\n");
            return false;
        }
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: �ڴ˴�ΪӦ�ó������Ϊ��д���롣
            wprintf(L"����: MFC ��ʼ��ʧ��\n");
            return false;
        }
        return true;
    }

    static void ShowError()//������
    {
        LPWSTR lpMessageBuf = nullptr;
        //strerror(error) ��׼C���Կ�
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&lpMessageBuf, 0, nullptr);
        OutputDebugString(lpMessageBuf);
        LocalFree(lpMessageBuf);
    }

//����˿�����������: ��һ�ַ���
//����������ʱ��, �����ܷ�����(�����Ѿ���ע�����ע��)�������û���Ȩ���й�, ������������Ȩ���ǹ���Ա����, �������û���Ȩ������ͨ�û�, �򲻻���������
//���������Ի���������Ӱ��, �������dll(��̬��), ���������ʧ��
//��Ҫʹ�ö�̬��, ������Щdll���ļ���system32(!!!����64λ����)����sysWOW64(!!!����32λ����)����
    static bool WriteRegisterTable(const CString& strPath)//��һ�ֿ���������ʽ, ͨ��ע���, �����ڿ��������û����������
    {
        CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");

        ////�����ӷ�ʽ
        //char sSys[MAX_PATH] = "";
        //char sPath[MAX_PATH] = "";
        //std::string strExe = "\\RemoteCtrl.exe ";
        //GetCurrentDirectoryA(MAX_PATH, sPath);
        //GetSystemDirectoryA(sSys, sizeof(sSys));
        //std::string strCmd = "mklink " + std::string(sSys) + strExe + std::string(sPath) + strExe;
        //system(strCmd.c_str());

        //��������.exe��ʽ
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(nullptr, sPath, MAX_PATH);
        bool ret = CopyFile(sPath, strPath, FALSE);
        if (ret == false)
        {
            MessageBox(nullptr, _T("�����ļ�ʧ��, �Ƿ�Ȩ�޲���?\r\n"), _T("����"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        HKEY hKey = nullptr;
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_WRITE | KEY_WOW64_64KEY, &hKey);//��ע����µ�ָ��·��
        if (ret != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("�����Զ���������ʧ��! �Ƿ�Ȩ�޲���?\r\n��������ʧ��!"), _T("����"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        //CString strPath = CString(_T("%SystemRoot%\\SysWOW64\\RemoteCtrl.exe"));
        ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));//��ӵ�ע����ָ��·����
        if (ret != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("�����Զ���������ʧ��! �Ƿ�Ȩ�޲���?\r\n��������ʧ��!"), _T("����"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        RegCloseKey(hKey);
        return true;
    }

    static bool WriteStartupDir(const CString& strPath)//�ڶ��ֿ���������ʽ, д�������ļ���, �������û����������
    {
       /* CString strCmd = GetCommandLine();
        strCmd.Replace(_T("\""), _T(""));
        bool ret = CopyFile(strCmd, strPath, FALSE);*/
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(nullptr, sPath, MAX_PATH);
        bool ret = CopyFile(sPath, strPath, FALSE);
        if (ret == false)
        {
            MessageBox(nullptr, _T("�����ļ�ʧ��, �Ƿ�Ȩ�޲���?\r\n"), _T("����"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        return true;
    }


    static bool RunAsAdmin()//ʹ�ù���ԱȨ�޴������� 
    {
        //��Ҫ�ڱ��ز�����: ���ù���Ա�˻�, ���ÿ�����ֻ�ܵ�¼���ؿ���̨

        ////��ȡ����ԱȨ��
        //HANDLE hToken = nullptr;
        //bool ret = LogonUser(L"Administrator", NULL, NULL, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken);
        //if (!ret)
        //{
        //    MessageBox(nullptr, _T("����Ա�û�����ʧ��"), _T("�������"), 0);
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
            MessageBox(nullptr, sPath, _T("��������ʧ��"), 0);
            ShowError();
            return false;
        }
        //MessageBox(nullptr, sPath, _T("�������̳ɹ�"), 0);
        WaitForSingleObject(pi.hProcess, INFINITE);//�ȴ��������ӽ��̽���
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
};



