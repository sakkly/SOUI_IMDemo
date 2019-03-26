// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	

#include "FileHelper.h"
#include "MenuWrapper.h"

#include "extend.ctrls/imre/ImgProvider.h"

#define  MAX_FILESIZE			4*1024*1024			//可发送文件最大限制
#define  MAX_IMAGESIZE			4*1024*1024			//图片最大限制

#include "CWindowEnumer.h"
	
CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_bLayoutInited = FALSE;

	m_bSnapshotHideWindow = false;
	m_bResultEmpty = true;

	m_pAdapterSearchResult = NULL;

	SNotifyCenter::getSingleton().addEvent(EVENTID(EventSnapshotFinish));
}

CMainDlg::~CMainDlg()
{
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetMsgHandled(FALSE);
	return 0;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	m_bLayoutInited = TRUE;
	//SShellNotifyIcon *notify = FindChildByID2<SShellNotifyIcon>(110);
	//notify->ShowNotify(L"Hello SOUI",L"这可能是一个提示");

	if (!ImageProvider::IsExist(L"default_portrait"))
	{
		SAntialiasSkin* pSkin = new SAntialiasSkin();
		pSkin->SetRound(TRUE);

		if (pSkin->LoadFromFile(L"default_res\\default_portrait.png"))
			ImageProvider::Insert(L"default_portrait", pSkin);
		else
			delete pSkin;
	}

	::DragAcceptFiles(m_hWnd, TRUE);
	::RegisterDragDrop(m_hWnd, GetDropTarget());

	SImageButton* pBtnMessage = FindChildByName2<SImageButton>(L"btn_message");
	SImageButton* pBtnContact = FindChildByName2<SImageButton>(L"btn_contact");
	SImageButton* pBtnFavorites = FindChildByName2<SImageButton>(L"btn_favorites");
	SASSERT(pBtnMessage);
	SASSERT(pBtnContact);
	SASSERT(pBtnFavorites);

	pBtnMessage->SetCheck(TRUE);
	pBtnContact->SetCheck(FALSE);
	pBtnFavorites->SetCheck(FALSE);

	SListView* pLasttalkList = FindChildByName2<SListView>(L"lv_list_lasttalk");
	SASSERT(pLasttalkList);
	pLasttalkList->EnableScrollBar(SSB_HORZ, FALSE);

	m_pAdapterLasttalk = new CAdapter_MessageList(pLasttalkList, this);
	pLasttalkList->SetAdapter(m_pAdapterLasttalk);
	m_pAdapterLasttalk->Release();

	SListView* pSearchResult = FindChildByName2<SListView>(L"lv_search_result");
	SASSERT(pSearchResult);
	pSearchResult->EnableScrollBar(SSB_HORZ, FALSE);

	m_pAdapterSearchResult = new CAdapter_SearchResult(pSearchResult, this);
	pSearchResult->SetAdapter(m_pAdapterSearchResult);
	m_pAdapterSearchResult->Release();

	STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
	SASSERT(pChatTab);
	//添加测试数据
	m_pAdapterLasttalk->AddItem(0, "file_helper");
	{//文件服务page
		SStringW sstrPage;
		sstrPage.Format(L"<page title='%s'><include src='layout:XML_PAGE_COMMON_FILEHELPER'/></page>", L"file_helper");
		pChatTab->InsertItem(sstrPage);

		SWindow* pPage = pChatTab->GetPage(L"file_helper", TRUE);
		SASSERT(pPage);
		SImRichEdit* pRecvRichedit = pPage->FindChildByName2<SImRichEdit>(L"recv_richedit");
		SImRichEdit* pSendRichedit = pPage->FindChildByName2<SImRichEdit>(L"send_richedit");
		SASSERT(pRecvRichedit);
		SASSERT(pSendRichedit);

		SUBSCRIBE(pSendRichedit, EVT_RE_OBJ, CMainDlg::OnSendRichEditObjEvent);
		SUBSCRIBE(pSendRichedit, EVT_RE_NOTIFY, CMainDlg::OnSendRichEditEditorChange);
		SUBSCRIBE(pSendRichedit, EVT_RE_QUERY_ACCEPT, CMainDlg::OnSendRichEditAcceptData);
		SUBSCRIBE(pSendRichedit, EVT_CTXMENU, CMainDlg::OnSendRichEditMenu);

		SUBSCRIBE(pRecvRichedit, EVT_RE_OBJ, CMainDlg::OnRecvRichEditObjEvent);
		SUBSCRIBE(pRecvRichedit, EVT_RE_SCROLLBAR, CMainDlg::OnRecvRichEditScrollEvent);
		SUBSCRIBE(pRecvRichedit, EVT_RE_QUERY_ACCEPT, CMainDlg::OnRecvRichEditAcceptData);
		SUBSCRIBE(pRecvRichedit, EVT_CTXMENU, CMainDlg::OnRecvRichEditMenu);
	}
	m_pAdapterLasttalk->AddItem(3, "page_dyh");
	{
		SStringW sstrPage;
		sstrPage.Format(L"<page title='%s'><include src='layout:XML_PAGE_DYH'/></page>", L"page_dyh");
		pChatTab->InsertItem(sstrPage);
	}
	m_pAdapterLasttalk->AddItem(4, "page_news");
	{
		SStringW sstrPage;
		sstrPage.Format(L"<page title='%s'><include src='layout:XML_PAGE_NEWS'/></page>", L"page_news");
		pChatTab->InsertItem(sstrPage);
	}
	m_pAdapterLasttalk->AddItem(5, "page_gzh");
	{
		SStringW sstrPage;
		sstrPage.Format(L"<page title='%s'><include src='layout:XML_PAGE_GZH'/></page>", L"page_gzh");
		pChatTab->InsertItem(sstrPage);
	}


	SStatic* pCurName = FindChildByName2<SStatic>(L"page_name");
	SASSERT(pCurName);
	pCurName->SetVisible(FALSE);

	SImageButton* pImgBtnMore = FindChildByName2<SImageButton>(L"btn_more");
	SASSERT(pImgBtnMore);
	pImgBtnMore->SetVisible(FALSE);


	STreeView * pTreeView = FindChildByName2<STreeView>("tv_Friend");
	if (pTreeView)
	{
		m_pTreeViewAdapter = new CContactTreeViewAdapter(this);
		pTreeView->SetAdapter(m_pTreeViewAdapter);
		m_pTreeViewAdapter->Release();
	}
	return 0;
}
//TODO:消息映射
void CMainDlg::OnClose()
{
	CSimpleWnd::DestroyWindow();
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	if (!m_bLayoutInited) return;
	
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if(!pBtnMax || !pBtnRestore) return;
	
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}


//演示如何响应菜单事件
void CMainDlg::OnCommand(UINT uNotifyCode, int nID, HWND wndCtl)
{
	if (uNotifyCode == 0)
	{
		switch (nID)
		{
		case 6:
			PostMessage(WM_CLOSE);
			break;
		default:
			break;
		}
	}
}


void CMainDlg::OnBnClickMessage()
{
	SImageButton* pBtnMessage = FindChildByName2<SImageButton>(L"btn_message");
	SImageButton* pBtnContact = FindChildByName2<SImageButton>(L"btn_contact");
	SImageButton* pBtnFavorites = FindChildByName2<SImageButton>(L"btn_favorites");
	SASSERT(pBtnMessage);
	SASSERT(pBtnContact);
	SASSERT(pBtnFavorites);

	pBtnMessage->SetCheck(TRUE);
	pBtnContact->SetCheck(FALSE);
	pBtnFavorites->SetCheck(FALSE);

	STabCtrl* pLeftListTab = FindChildByName2<STabCtrl>(L"leftlist_tabctrl");
	SASSERT(pLeftListTab);
	pLeftListTab->SetCurSel(L"lasttalk_page", TRUE);

	if ("" != m_LasttalkCurSel.m_strID)
	{
		SStatic* pCurName = FindChildByName2<SStatic>(L"page_name");
		SASSERT(pCurName);
		pCurName->SetVisible(TRUE);
	}

	SImageButton* pImgBtnMore = FindChildByName2<SImageButton>(L"btn_more");
	SASSERT(pImgBtnMore);
	if (3 == m_LasttalkCurSel.m_nType ||
		4 == m_LasttalkCurSel.m_nType ||
		5 == m_LasttalkCurSel.m_nType)
	{
		pImgBtnMore->SetVisible(FALSE);
	}
	else
		pImgBtnMore->SetVisible(TRUE);

	if ("" != m_LasttalkCurSel.m_strID)
	{
		SStringW sstrID = S_CA2W(m_LasttalkCurSel.m_strID.c_str());
		STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
		SASSERT(pChatTab);
		pChatTab->SetCurSel(sstrID, TRUE);
	}

	SStatic* pCurName = FindChildByName2<SStatic>(L"page_name");
	SASSERT(pCurName);
	pCurName->SetVisible(TRUE);

	GetRoot()->Invalidate();
}

void CMainDlg::OnBnClickContact()
{
	SImageButton* pBtnMessage = FindChildByName2<SImageButton>(L"btn_message");
	SImageButton* pBtnContact = FindChildByName2<SImageButton>(L"btn_contact");
	SImageButton* pBtnFavorites = FindChildByName2<SImageButton>(L"btn_favorites");
	SASSERT(pBtnMessage);
	SASSERT(pBtnContact);
	SASSERT(pBtnFavorites);

	pBtnMessage->SetCheck(FALSE);
	pBtnContact->SetCheck(TRUE);
	pBtnFavorites->SetCheck(FALSE);

	STabCtrl* pLeftListTab = FindChildByName2<STabCtrl>(L"leftlist_tabctrl");
	SASSERT(pLeftListTab);
	pLeftListTab->SetCurSel(L"contact_page", TRUE);

	STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
	SASSERT(pChatTab);
	pChatTab->SetCurSel(0);

	SImageButton* pImgBtnMore = FindChildByName2<SImageButton>(L"btn_more");
	SASSERT(pImgBtnMore);
	pImgBtnMore->SetVisible(FALSE);

	SStatic* pCurName = FindChildByName2<SStatic>(L"page_name");
	SASSERT(pCurName);
	pCurName->SetVisible(FALSE);

	GetRoot()->Invalidate();
}

void CMainDlg::OnBnClickFavorites()
{
	SImageButton* pBtnMessage = FindChildByName2<SImageButton>(L"btn_message");
	SImageButton* pBtnContact = FindChildByName2<SImageButton>(L"btn_contact");
	SImageButton* pBtnFavorites = FindChildByName2<SImageButton>(L"btn_favorites");
	SASSERT(pBtnMessage);
	SASSERT(pBtnContact);
	SASSERT(pBtnFavorites);

	pBtnMessage->SetCheck(FALSE);
	pBtnContact->SetCheck(FALSE);
	pBtnFavorites->SetCheck(TRUE);

	STabCtrl* pLeftListTab = FindChildByName2<STabCtrl>(L"leftlist_tabctrl");
	SASSERT(pLeftListTab);
	pLeftListTab->SetCurSel(L"favorites_page", TRUE);

	STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
	SASSERT(pChatTab);
	pChatTab->SetCurSel(0);

	SImageButton* pImgBtnMore = FindChildByName2<SImageButton>(L"btn_more");
	SASSERT(pImgBtnMore);
	pImgBtnMore->SetVisible(FALSE);

	SStatic* pCurName = FindChildByName2<SStatic>(L"page_name");
	SASSERT(pCurName);
	pCurName->SetVisible(FALSE);

	GetRoot()->Invalidate();
}

void CMainDlg::OnBnClickMenu()
{
	//TODO:
}

void CMainDlg::OnBnClickSearchCancel()
{
	STabCtrl* pTab = FindChildByName2<STabCtrl>(L"leftlist_tabctrl");
	SASSERT(pTab);
	pTab->SetCurSel(m_nOldCurSel);

	SEdit *pEdit = FindChildByName2<SEdit>(L"search_edit");
	SASSERT(pEdit);
	pEdit->SetWindowText(L"");
	m_bResultEmpty = true;
}

void CMainDlg::MessageListItemClick(int nType, const std::string& strID)
{
	STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
	SASSERT(pChatTab);

	SStringW sstrID = S_CA2W(strID.c_str());
	pChatTab->SetCurSel(sstrID, TRUE);

	SStatic* pCurName = FindChildByName2<SStatic>(L"page_name");
	SASSERT(pCurName);

	SStringW sstrName = L"";
	switch (nType)
	{
	case 0://filehelper
		sstrName = L"文件传输助手";
		break;
	case 1://personal
		{
			std::string strName = "";
			PersonalsMap::iterator iter = CGlobalUnits::GetInstance()->m_mapPersonals.find(strID);
			if (iter != CGlobalUnits::GetInstance()->m_mapPersonals.end())
				strName = iter->second.m_strName;
			else
				strName = strID;

			sstrName = S_CA2W(strName.c_str());
		}
		break;
	case 2://group
		{
			std::string strName = "";
			GroupsMap::iterator iter = CGlobalUnits::GetInstance()->m_mapGroups.find(strID);
			if (iter != CGlobalUnits::GetInstance()->m_mapGroups.end())
				strName = iter->second.m_strGroupName;
			else
				strName = strID;

			sstrName = S_CA2W(strName.c_str());
		}
		break;
	case 3://订阅号
		sstrName = L"订阅号";
		break;
	case 4://新闻
		sstrName = L"新闻";
		break;
	case 5://公众号
		sstrName = L"公众号";
		break;
	default:
		break;
	}
	pCurName->SetVisible(TRUE);
	pCurName->SetWindowText(sstrName);
	pCurName->Invalidate();

// 	SImageButton* pImgBtnMore = FindChildByName2<SImageButton>(L"btn_more");
// 	SASSERT(pImgBtnMore);
// 	pImgBtnMore->SetVisible(TRUE);
// 	pImgBtnMore->Invalidate();

	m_LasttalkCurSel.m_nType = nType;
	m_LasttalkCurSel.m_strID = strID;

	SImageButton* pImgBtnMore = FindChildByName2<SImageButton>(L"btn_more");
	SASSERT(pImgBtnMore);
	if (3 == m_LasttalkCurSel.m_nType ||
		4 == m_LasttalkCurSel.m_nType ||
		5 == m_LasttalkCurSel.m_nType)
	{
		pImgBtnMore->SetVisible(FALSE);
	}
	else
		pImgBtnMore->SetVisible(TRUE);
	
	GetRoot()->Invalidate();
}

void CMainDlg::MessageListItemRClick(int nType, const std::string& strID)
{
	//Show R_Menu
}

void CMainDlg::ContactItemClick(int nGID, const std::string& strID)
{
	int i = 0;
}

void CMainDlg::ContactItemDBClick(int nGID, const std::string& strID)
{
	/*
	*	根据GID区分
	*	1、新朋友
	*	2、公众号
	*	3、订阅号
	*	4、群聊
	*	5、个人
	*/
	if (1 == nGID || 2 == nGID || 3 == nGID)	return;    //不处理新朋友，公众号，订阅号的双击事件

	if (4 == nGID)		//group
		m_pAdapterLasttalk->AddItem(2, strID);
	else if (5 == nGID)	//personal
		m_pAdapterLasttalk->AddItem(1, strID);

	SStringW sstrID = S_CA2W(strID.c_str());
	STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
	SASSERT(pChatTab);
	SStringW sstrPage;	
	if (4 == nGID)		//group
		sstrPage.Format(L"<page title='%s'><include src='layout:XML_PAGE_COMMON_GROUP'/></page>", sstrID);
	else if (5 == nGID)	//personal
		sstrPage.Format(L"<page title='%s'><include src='layout:XML_PAGE_COMMON_PERSONAL'/></page>", sstrID);
	pChatTab->InsertItem(sstrPage);

	SWindow* pPage = pChatTab->GetPage(sstrID, TRUE);
	SASSERT(pPage);
	SImRichEdit* pRecvRichedit = pPage->FindChildByName2<SImRichEdit>(L"recv_richedit");
	SImRichEdit* pSendRichedit = pPage->FindChildByName2<SImRichEdit>(L"send_richedit");
	SASSERT(pRecvRichedit);
	SASSERT(pSendRichedit);

	SUBSCRIBE(pSendRichedit, EVT_RE_OBJ, CMainDlg::OnSendRichEditObjEvent);
	SUBSCRIBE(pSendRichedit, EVT_RE_NOTIFY, CMainDlg::OnSendRichEditEditorChange);
	SUBSCRIBE(pSendRichedit, EVT_RE_QUERY_ACCEPT, CMainDlg::OnSendRichEditAcceptData);
	SUBSCRIBE(pSendRichedit, EVT_CTXMENU, CMainDlg::OnSendRichEditMenu);

	SUBSCRIBE(pRecvRichedit, EVT_RE_OBJ, CMainDlg::OnRecvRichEditObjEvent);
	SUBSCRIBE(pRecvRichedit, EVT_RE_SCROLLBAR, CMainDlg::OnRecvRichEditScrollEvent);
	SUBSCRIBE(pRecvRichedit, EVT_RE_QUERY_ACCEPT, CMainDlg::OnRecvRichEditAcceptData);
	SUBSCRIBE(pRecvRichedit, EVT_CTXMENU, CMainDlg::OnRecvRichEditMenu);

	if (4 == nGID)		//group
	{
		m_LasttalkCurSel.m_nType = 2;
		m_LasttalkCurSel.m_strID = strID;
	}
	else if (5 == nGID)	//personal
	{
		m_LasttalkCurSel.m_nType = 1;
		m_LasttalkCurSel.m_strID = strID;
	}
	OnBnClickMessage();
	MessageListItemClick(m_LasttalkCurSel.m_nType, m_LasttalkCurSel.m_strID = strID);

	//如果不想添加在列表后边，可选择添加到列表开头
	//m_pAdapterLasttalk->MoveItemToTop(strID);	

	//将滚动条滚动到刚添加的项
	m_pAdapterLasttalk->EnsureVisable(m_LasttalkCurSel.m_strID = strID);
	//设置将最新添加的项选中
	m_pAdapterLasttalk->SetCurSel(m_LasttalkCurSel.m_strID = strID);
}

void CMainDlg::ContactItemRClick(int nGID, const std::string& strID)
{
	int i = 0;
}

void CMainDlg::SearchResultItemDBClick(int nType, const std::string& strID)
{
	//4 group/5 personal
	if (nType == 1)
		ContactItemDBClick(5, strID);
	else if (nType == 2)
		ContactItemDBClick(4, strID);

	STabCtrl* pTab = FindChildByName2<STabCtrl>(L"leftlist_tabctrl");
	SASSERT(pTab);
	pTab->SetCurSel(m_nOldCurSel);

	SEdit *pEdit = FindChildByName2<SEdit>(L"search_edit");
	SASSERT(pEdit);
	pEdit->SetWindowText(L"");

	m_bResultEmpty = true;
}


void CMainDlg::OnBnClickSend()
{
	STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
	SASSERT(pChatTab);

	SStringW sstrID = S_CA2W(m_LasttalkCurSel.m_strID.c_str());
	SWindow* pPage = pChatTab->GetPage(sstrID, TRUE);
	SASSERT(pPage);
	SImRichEdit* pRecvRichedit = pPage->FindChildByName2<SImRichEdit>(L"recv_richedit");
	SImRichEdit* pSendRichedit = pPage->FindChildByName2<SImRichEdit>(L"send_richedit");
	SASSERT(pRecvRichedit);
	SASSERT(pSendRichedit);

	CHARRANGE chr = {0, -1};
	SStringT strContent = pSendRichedit->GetSelectedContent(&chr);
	pugi::xml_document doc;
	if (!doc.load_buffer(strContent, strContent.GetLength() * sizeof(WCHAR)))
		return;
	strContent.Empty();

	std::vector<SStringW> vecContent;
	pugi::xml_node node = doc.child(L"RichEditContent").first_child();
	for (; node; node = node.next_sibling())
	{
		const wchar_t* pNodeName = node.name();
		if (wcscmp(RichEditText::GetClassName(), pNodeName) == 0)			//文本
		{
			SStringW sstrContent = node.text().get();
			SStringW sstrText;
			sstrText.Format(L"<text font-size=\"10\" font-face=\"微软雅黑\" color=\"#000000\"><![CDATA[%s]]></text>", sstrContent);
			vecContent.push_back(sstrText);
		}
		else if (wcscmp(RichEditImageOle::GetClassName(), pNodeName) == 0)	//图片
		{
			SStringW sstrImgPath = node.attribute(L"path").as_string();
			SStringW sstrImg;
			sstrImg.Format(L"<img subid=\"%s\" id=\"%s\" type=\"normal_img\" encoding=\"\" show-magnifier=\"1\" path=\"%s\"/>", L"", L"", sstrImgPath);
			vecContent.push_back(sstrImg);
		}
		else if (wcscmp(RichEditMetaFileOle::GetClassName(), pNodeName) == 0)	//文件
		{
			SStringW sstrFilePath = node.attribute(L"file").as_string();
			AddBackFileMessage(pRecvRichedit, sstrFilePath);
		}
		else
		{
			//其他未知消息类型
		}
	}

	if (!vecContent.empty())
		AddBackGeneralMessage(pRecvRichedit, vecContent);

	pSendRichedit->Clear();
}

bool CMainDlg::OnSendRichEditAcceptData(SOUI::EventArgs* pEvt)
{
	EventQueryAccept * pev = (EventQueryAccept*)pEvt;
	if (pev->Conv->GetAvtiveFormat() == CF_HDROP)
	{
		::SetForegroundWindow(m_hWnd);
		RichFormatConv::DropFiles files = pev->Conv->GetDropFiles();
		DragDropFiles(files);
		return true;
	}
	return true;
}

bool CMainDlg::OnSendRichEditEditorChange(SOUI::EventArgs* pEvt)
{
	EventRENotify* pReNotify = (EventRENotify*)pEvt;
	if (pReNotify)
	{
		//处理输入框内容更改
	}
	return false;
}

bool CMainDlg::OnSendRichEditMenu(SOUI::EventArgs* pEvt)
{
	//发送框右键菜单事件响应
	SStringW sstrID = S_CA2W(m_LasttalkCurSel.m_strID.c_str());

	STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
	SASSERT(pChatTab);
	SWindow* pPage = pChatTab->GetPage(sstrID, TRUE);
	SASSERT(pPage);
	SImRichEdit* pSendRichEdit = pPage->FindChildByName2<SImRichEdit>(L"send_richedit");
	SASSERT(pSendRichEdit);

	CHARRANGE chrSel;
	pSendRichEdit->GetSel(&chrSel.cpMin, &chrSel.cpMax);
	RichEditOleBase* pOle = pSendRichEdit->GetSelectedOle();

	MenuWrapper menu(L"menu_send_richeidt", L"SMENU");
	BOOL enableCopy = (chrSel.cpMax - chrSel.cpMin) >= 1;
	menu.AddMenu(_T("复制"), 1, enableCopy, FALSE);
	menu.AddMenu(_T("剪切"), 2, enableCopy, FALSE);
	menu.AddMenu(_T("粘贴"), 3, TRUE, FALSE);

	int ret = 0;
	POINT pt;
	::GetCursorPos(&pt);

	ret = menu.ShowMenu(TPM_RETURNCMD, pt.x, pt.y, m_hWnd);
	switch (ret)
	{
	case 1:
		pSendRichEdit->Copy();
		break;

	case 2:
		pSendRichEdit->Cut();
		break;

	case 3:
		pSendRichEdit->Paste();
		break;
	default:
		break;
	}

	return true;
}

bool CMainDlg::OnSendRichEditObjEvent(SOUI::EventArgs* pEvt)
{
	EventRichEditObj * pev = (EventRichEditObj*)pEvt;
	switch (pev->SubEventId)
	{
	case DBLCLICK_IMAGEOLE:
		{
			RichEditImageOle * pImageOle = sobj_cast<RichEditImageOle>(pev->RichObj);
			ShellExecute(NULL, _T("open"), pImageOle->GetImagePath(), NULL, NULL, SW_SHOW);
		}
		break;
	default:
		break;
	}
	return false;
}

bool CMainDlg::OnRecvRichEditAcceptData(SOUI::EventArgs *pEvt)
{
	EventQueryAccept* pev = (EventQueryAccept*)pEvt;
	if (pev)
	{
		//一般没处理需求
	}
	return true;
}

bool CMainDlg::OnRecvRichEditMenu(SOUI::EventArgs *pEvt)
{
	EventCtxMenu* pev = static_cast<EventCtxMenu*>(pEvt);
	if (pev)
	{
		STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
		SASSERT(pChatTab);

		SStringW sstrID = S_CA2W(m_LasttalkCurSel.m_strID.c_str());
		SWindow* pPage = pChatTab->GetPage(sstrID, TRUE);
		SImRichEdit* pRecvRichedit = pPage->FindChildByName2<SImRichEdit>(L"recv_richedit");
		SASSERT(pRecvRichedit);

		CHARRANGE selRange;
		pRecvRichedit->GetSel(&selRange.cpMin, &selRange.cpMax);
		int selectedCount = selRange.cpMax - selRange.cpMin;

		// 如果鼠标没有落在选中区域上，需要取消选中
		if (selectedCount > 0)
		{
			int cp = pRecvRichedit->CharFromPos(pev->pt);
			if (cp < selRange.cpMin || cp > selRange.cpMax)
			{
				pRecvRichedit->SetSel(cp, cp);
				selectedCount = 0;
			}
		}

		//对于右键在选择区域上的各种判断
		RichEditObj * pHitTestObj = pRecvRichedit->HitTest(pev->pt);
		RichEditObj * pObj = pHitTestObj;
		RichEditContent * pContent = sobj_cast<RichEditContent>(pObj);
		RichEditImageOle* pImageOle = NULL;
		RichEditFileOle* pFileOle = NULL;

		MenuWrapper menu(L"menu_recv_richeidt", L"SMENU");

		//弹出菜单
		int ret = 0;
		POINT pt;
		::GetCursorPos(&pt);

		if (pContent == NULL || pHitTestObj == NULL)
		{
			//
		}
		else if (pHitTestObj->IsClass(RichEditBkElement::GetClassName()) && pHitTestObj->GetData() == _T("avatar"))
		{
			 //
		}
		else if (pHitTestObj->IsClass(RichEditImageOle::GetClassName()))
		{
			//
		}
		else if (pHitTestObj->IsClass(RichEditFileOle::GetClassName()))
		{
			//
		}
		else if (pHitTestObj->IsClass(RichEditReminderOle::GetClassName()))
		{
			//
		}
		else if (pHitTestObj->IsClass(RichEditBkElement::GetClassName()) && pHitTestObj->GetData() == _T("bubble"))
		{
			//
		}
		else
		{
			//FillRClickNothingMenu(menu);
			//ret = menu.ShowMenu(TPM_RETURNCMD, pt.x, pt.y, m_hWnd);
		}

		//处理菜单消息
		switch (ret)
		{
		case 0:
			break;
		default:
			break;
		}
	}
	return true;
}

bool CMainDlg::OnRecvRichEditObjEvent(SOUI::EventArgs *pEvt)
{
	return true;
}

bool CMainDlg::OnRecvRichEditScrollEvent(SOUI::EventArgs *pEvt)
{
	return true;
}

void CMainDlg::DragDropFiles(RichFormatConv::DropFiles& files)
{
	SStringW sstrID = S_CA2W(m_LasttalkCurSel.m_strID.c_str());

	STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
	SASSERT(pChatTab);
	SWindow* pPage = pChatTab->GetPage(sstrID, TRUE);
	SASSERT(pPage);
	SImRichEdit* pSendRichEdit = pPage->FindChildByName2<SImRichEdit>(L"send_richedit");
	SASSERT(pSendRichEdit);

	RichFormatConv::DropFiles::iterator iter = files.begin();
	for (; iter != files.end(); ++iter)
	{
		SStringW strFile = *iter;
		int nFileSize;
		FILE* fp = _wfopen(strFile, L"rb");
		if (fp)
		{
			fseek(fp, 0L, SEEK_END);
			nFileSize = ftell(fp);
			rewind(fp);
			fclose(fp);
		}
		else
			return;

		//可对发送的文件大小做个限制
// 		if(nFileSize >= MAX_FILESIZE)
// 		{
// 			::MessageBox(this->m_hWnd, L"仅支持4M以下的文件，该文件大于4M", L"提示", MB_OK);
// 			return;
// 		}

		SStringW sstrContent;
		sstrContent.Format(L"<RichEditContent><metafile file=\"%s\" /></RichEditContent>", *iter);
		pSendRichEdit->InsertContent(sstrContent, RECONTENT_CARET);
	}

	::SetFocus(m_hWnd);
	pSendRichEdit->SetFocus();
	pSendRichEdit->GetContainer()->OnShowCaret(TRUE);
}

void CMainDlg::OnBnClickChatEmotion()
{
	//
}

void CMainDlg::OnBnClickChatImage()
{
	SStringW strFile;
	CFileDialogEx openDlg(TRUE,_T("图片"),0,6,
		_T("图片文件\0*.gif;*.bmp;*.jpg;*.png\0\0"));
	if (openDlg.DoModal() == IDOK)
	{
		strFile = openDlg.m_szFileName;

		int nFileSize;
		FILE* fp = _wfopen(strFile, L"rb");
		if (fp)
		{
			fseek(fp, 0L, SEEK_END);
			nFileSize = ftell(fp);
			rewind(fp);
			fclose(fp);
		}
		else
			return;

		//发送图片限制大小
// 		if(nFileSize >= MAX_FILESIZE)
// 		{
// 			::MessageBox(this->m_hWnd, L"仅支持4M以下的图片，该图片大于4M", L"提示", MB_OK);
// 			return;
// 		}

		SStringW str;
		str.Format(L"<RichEditContent>"
			L"<para break=\"0\" disable-layout=\"1\">"
			L"<img type=\"normal_img\" path=\"%s\" id=\"zzz\" max-size=\"\" minsize=\"\" scaring=\"1\" cursor=\"hand\" />"
			L"</para>"
			L"</RichEditContent>", strFile);

		STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
		SASSERT(pChatTab);

		SStringW sstrID = S_CA2W(m_LasttalkCurSel.m_strID.c_str());

		SWindow* pPage = pChatTab->GetPage(sstrID, TRUE);
		SImRichEdit* pSendRichedit = pPage->FindChildByName2<SImRichEdit>(L"send_richedit");
		pSendRichedit->InsertContent(str, RECONTENT_CARET);
	}
}

void CMainDlg::OnBnClickChatFile()
{
	SStringW strFile;
	CFileDialogEx openDlg(TRUE,_T("文件"),0,6,
		_T("文件\0*.*\0\0"));
	if (openDlg.DoModal() == IDOK)
	{
		strFile = openDlg.m_szFileName;
		int nFileSize;
		FILE* fp = _wfopen(strFile, L"rb");
		if (fp)
		{
			fseek(fp, 0L, SEEK_END);
			nFileSize = ftell(fp);
			rewind(fp);
			fclose(fp);
		}
		else
			return;

		//发送图片限制大小
// 		if(nFileSize >= MAX_FILESIZE)
// 		{
// 			::MessageBox(this->m_hWnd, L"仅支持4M以下的文件，该文件大于4M", L"提示", MB_OK);
// 			return;
// 		}

		SStringW str;
		str.Format(L"<RichEditContent>"
			L"<metafile file=\"%s\" />"
			L"</RichEditContent>", strFile);

		STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
		SASSERT(pChatTab);

		SStringW sstrID = S_CA2W(m_LasttalkCurSel.m_strID.c_str());

		SWindow* pPage = pChatTab->GetPage(sstrID, TRUE);
		SImRichEdit* pSendRichedit = pPage->FindChildByName2<SImRichEdit>(L"send_richedit");
		pSendRichedit->InsertContent(str, RECONTENT_CARET);
	}
}

void CMainDlg::OnBnClickChatCapture()
{
	if (m_bSnapshotHideWindow)
	{
		this->ShowWindow(SW_HIDE);
	}

	CSnapshotDlg dlg;
	CWindowEnumer::EnumAllTopWindow();
	dlg.DoModal(NULL);
}

void CMainDlg::OnBnClickSettingCapture()
{
	//点击截图旁的箭头
	MenuWrapper menu(L"menu_snapshot_arrow", L"SMENU");
	menu.AddMenu(L"屏幕截图", 2001, TRUE, FALSE);
	menu.AddMenu(L"截图隐藏当前窗口", 2002, TRUE, FALSE);

	MenuItemWrapper* pItemCapture = menu.GetMenuItemById(2001);
	MenuItemWrapper* pItemHideWindow = menu.GetMenuItemById(2002);

	if (m_bSnapshotHideWindow)
		pItemHideWindow->SetCheck(TRUE);

	int ret = 0;
	POINT pt;
	::GetCursorPos(&pt);
	ret = menu.ShowMenu(TPM_RETURNCMD, pt.x, pt.y, m_hWnd);
	switch (ret)
	{
	case 2001:
			OnBnClickChatCapture();
		break;

	case 2002:
		{
			if (m_bSnapshotHideWindow)
				m_bSnapshotHideWindow = false;
			else
				m_bSnapshotHideWindow = true;
		}
		break;

	default:
		break;
	}
}

void CMainDlg::OnBnClickChatHistory()
{
	//
}

void CMainDlg::AddBackGeneralMessage(SImRichEdit* pRecvRichEdit, const std::vector<SStringW>& vecContents)
{
	//统一使用右侧布局
	LPCWSTR pEmpty;
	pEmpty = L"<para id=\"msgbody\" margin=\"0,0,0,0\" break=\"1\" simulate-align=\"1\">"		
		L""
		L"</para>";

	SStringW sstrResend;
	sstrResend = L"<bkele id=\"resend\" name=\"BkEleSendFail\" data=\"resend\" right-skin=\"skin.richedit_resend\" right-pos=\"{-15,[-16,@12,@12\" cursor=\"hand\" interactive=\"1\"/>";

	SStringW sstrMsg;
	for (int i = 0; i < vecContents.size(); i++)
	{
		sstrMsg += vecContents[i];
	}

	SStringW sstrContent;
	sstrContent.Format(
		L"<RichEditContent  type=\"ContentRight\" align=\"right\" auto-layout=\"1\">"
		L"<para break=\"1\" align=\"left\" />"
		L"<bkele data=\"avatar\" id=\"%s\" skin=\"%s\" left-pos=\"0,]-6,@40,@40\" right-pos=\"-50,]-9,@40,@40\" cursor=\"hand\" interactive=\"1\"/>"
		L"<para id=\"msgbody\" margin=\"65,0,35,0\" break=\"1\" simulate-align=\"1\">"		
		L"%s"
		L"</para>"
		L"<bkele data=\"bubble\" left-skin=\"skin_left_bubble\" right-skin=\"skin_right_otherbubble\" left-pos=\"50,{-9,[10,[10\" right-pos=\"{-10,{-9,-55,[10\" />"
		L"%s"
		L"</RichEditContent>",
		L"default_portrait", L"default_portrait", sstrMsg, pEmpty);

	pRecvRichEdit->InsertContent(sstrContent, RECONTENT_LAST);
	pRecvRichEdit->ScrollToBottom();

	RichEditBkElement* pResendEleObj = sobj_cast<RichEditBkElement>(pRecvRichEdit->GetElementById(L"resend"));
	if (pResendEleObj)
	{
		pResendEleObj->SetVisible(FALSE);
		pRecvRichEdit->Invalidate();
	}
}

void CMainDlg::AddBackFileMessage(SImRichEdit* pRecvRichEdit, const SStringW& sstrFilePath)
{
	//统一使用右侧布局
	LPCWSTR pEmpty;
	pEmpty = L"<para id=\"msgbody\" margin=\"0,0,0,0\" break=\"1\" simulate-align=\"1\">"		
		L""
		L"</para>";

	SStringW sstrResend;
	sstrResend = L"<bkele id=\"resend\" name=\"BkEleSendFail\" data=\"resend\" right-skin=\"skin.richedit_resend\" right-pos=\"{-15,[-16,@12,@12\" cursor=\"hand\" interactive=\"1\"/>";

	std::string strFilePath = S_CW2A(sstrFilePath);
	//获取文件大小
	FILE* fp = fopen(strFilePath.c_str(), "rb");
	SASSERT(fp);
	int nFileSize = 0;
	fseek(fp, 0L, SEEK_END);
	nFileSize = ftell(fp);
	rewind(fp);
	fclose(fp);

	//获取文件后缀名
	std::string strFileSuffix;
	std::string::size_type pos = strFilePath.find_last_of(".");
	if (pos != std::string::npos)
	{
		strFileSuffix = strFilePath.substr(pos + 1);
	}

	SStringW sstrFileType = S_CA2W(strFileSuffix.c_str());

	SStringW sstrFile;
	int nVisableLinks = 0x0010 | 0x0020;
	sstrFile.Format(L"<file id=\"file_file\" file-size=\"%d\" file-state=\"上传成功\" file-suffix=\"%s\" file-path=\"%s\" links=\"%d\"/>",
		nFileSize,
		sstrFileType,
		sstrFilePath,
		nVisableLinks);

	SStringW sstrContent;
	sstrContent.Format(
		L"<RichEditContent  type=\"ContentRight\" align=\"right\" auto-layout=\"1\">"
		L"<para break=\"1\" align=\"left\" />"
		L"<bkele data=\"avatar\" id=\"%s\" skin=\"%s\" left-pos=\"0,]-6,@40,@40\" right-pos=\"-50,]-10,@40,@40\" cursor=\"hand\" interactive=\"1\"/>"
		L"<para data=\"file_file\" id=\"msgbody\" margin=\"65,0,45,0\" break=\"1\" simulate-align=\"1\">"		
		L"%s"
		L"</para>"
		L"<bkele data=\"bubble\" left-skin=\"skin_left_bubble\" right-skin=\"skin_right_otherbubble\" left-pos=\"50,{-9,[10,[10\" right-pos=\"{-10,{-9,-55,[10\" />"
		L"%s"
		L"</RichEditContent>",
		L"default_portrait", L"default_portrait", sstrFile, pEmpty);

	pRecvRichEdit->InsertContent(sstrContent, RECONTENT_LAST);
	pRecvRichEdit->ScrollToBottom();

	RichEditBkElement* pResendEleObj = sobj_cast<RichEditBkElement>(pRecvRichEdit->GetElementById(L"resend"));
	if (pResendEleObj)
	{
		pResendEleObj->SetVisible(FALSE);
		pRecvRichEdit->Invalidate();
	}
}

void CMainDlg::AddBackSysMessage(SImRichEdit* pRecvRichEdit, const SStringW& sstrContent)
{
	//
}

bool CMainDlg::OnEditSearchSetFocus(EventArgs* pEvt)
{
	STabCtrl* pTab = FindChildByName2<STabCtrl>(L"leftlist_tabctrl");
	SASSERT(pTab);
	m_nOldCurSel = pTab->GetCurSel();
	pTab->SetCurSel(L"search_result", TRUE);

	return true;
}

bool CMainDlg::OnEditSearchKillFocus(EventArgs* pEvt)
{
	if (m_bResultEmpty)
	{
		STabCtrl* pTab = FindChildByName2<STabCtrl>(L"leftlist_tabctrl");
		SASSERT(pTab);
		pTab->SetCurSel(m_nOldCurSel);
	}

	return true;
}

bool CMainDlg::OnEditSearchChanged(EventArgs *e)
{
	EventRENotify *e2 = sobj_cast<EventRENotify>(e);
	if(e2->iNotify != EN_CHANGE) 
		return false;

	SEdit *pEdit = sobj_cast<SEdit>(e->sender);
	SStringW sstrInput = pEdit->GetWindowText();
	std::wstring wstrInput = sstrInput;

	std::vector<SEARCH_INFO> vecSearchResult;
	if (L"" != sstrInput)
	{
		m_bResultEmpty = false;
		if (!CGlobalUnits::GetInstance()->IsIncludeChinese(wstrInput))	//拼音搜索
		{
			SearchInfosMap::iterator iter = CGlobalUnits::GetInstance()->m_mapPinyinSearch.begin();
			for (; iter != CGlobalUnits::GetInstance()->m_mapPinyinSearch.end(); iter++)
			{
				std::wstring wstrName = iter->first;
				if (wstrName.find(wstrInput) != std::string::npos)
					vecSearchResult.push_back(SEARCH_INFO(iter->second.m_nType, iter->second.m_strID));
			}
			//去重
			sort(vecSearchResult.begin(), vecSearchResult.end());
			vecSearchResult.erase(unique(vecSearchResult.begin(), vecSearchResult.end()), vecSearchResult.end());
		}
		else	//汉字搜索
		{
			SearchInfosMap::iterator iter = CGlobalUnits::GetInstance()->m_mapChineseSearch.begin();
			for (; iter != CGlobalUnits::GetInstance()->m_mapChineseSearch.end(); iter++)
			{
				std::wstring wstrName = iter->first;
				if (wstrName.find(wstrInput) != std::string::npos)
					vecSearchResult.push_back(SEARCH_INFO(iter->second.m_nType, iter->second.m_strID));
			}
			//去重
			sort(vecSearchResult.begin(), vecSearchResult.end());
			vecSearchResult.erase(unique(vecSearchResult.begin(), vecSearchResult.end()), vecSearchResult.end());
		}

		m_pAdapterSearchResult->DeleteAllItem();
		for (int i = 0; i < vecSearchResult.size(); i++)
		{
			m_pAdapterSearchResult->AddItem(vecSearchResult[i].m_nType, vecSearchResult[i].m_strID);
		}
	}
	else
	{
		m_pAdapterSearchResult->DeleteAllItem();
		m_bResultEmpty = true;
	}

	return true;
}

bool CMainDlg::OnEventSnapshotFinish(EventArgs* e)
{
	if (m_bSnapshotHideWindow)
		this->ShowWindow(SW_SHOWNORMAL);
	
	STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
	SASSERT(pChatTab);

	SStringW sstrID = S_CA2W(m_LasttalkCurSel.m_strID.c_str());
	if (0 == m_LasttalkCurSel.m_nType || 1 == m_LasttalkCurSel.m_nType || 2 == m_LasttalkCurSel.m_nType)
	{
		SWindow* pPage = pChatTab->GetPage(sstrID, TRUE);
		SImRichEdit* pSendRichedit = pPage->FindChildByName2<SImRichEdit>(L"send_richedit");
		SASSERT(pSendRichedit);

		pSendRichedit->Paste();
		pSendRichedit->SetFocus();
	}
	return true;
}