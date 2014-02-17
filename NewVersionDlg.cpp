// NewVersionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ParNRar.h"
#include "NewVersionDlg.h"
#include "WebAccess.h"

#import "msxml.dll" named_guids 

extern CParNRarApp theApp;

IMPLEMENT_DYNAMIC(NewVersionDlg, CDialog)

NewVersionDlg::NewVersionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(NewVersionDlg::IDD, pParent)
{
}

NewVersionDlg::~NewVersionDlg()
{
}

void NewVersionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DETAILS, ctlDetails);
}



BEGIN_MESSAGE_MAP(NewVersionDlg, CDialog)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


void NewVersionDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	int iTotalTextLength = ctlDetails.GetWindowTextLength();
	ctlDetails.SetSel(iTotalTextLength, iTotalTextLength);
	BeginWaitCursor();
	CString sResult;
	CWebAccess webAccess;
	webAccess.Get("www.milow.net/site/projects/parnrar/pnrVersions.xml", sResult);

	if (sResult != "")
		{
		MSXML::IXMLDOMDocumentPtr domVersions;	

		if (CoCreateInstance( MSXML::CLSID_DOMDocument, 0, CLSCTX_INPROC_SERVER, MSXML::IID_IXMLDOMDocument, (void**)&domVersions ) != S_OK)
		{
			ctlDetails.ReplaceSel("Error: Failed to create DOMDocument object");
			goto CLEANUP;
		}
		domVersions->loadXML(sResult.GetBuffer(0));
		CString sXml = static_cast<char *>(domVersions->xml);
		if (sXml == "")
		{
			ctlDetails.ReplaceSel("Error: Failed to download version information");
			goto CLEANUP;
		}
		MSXML::IXMLDOMElementPtr eVer = domVersions->selectSingleNode("/Versions/Version");
		CString sVer = static_cast<char *>(_bstr_t(eVer->getAttribute("number")));
		if (sVer == theApp.GetMyVersion())
		{
			ctlDetails.ReplaceSel("You are currently running the latest version of Par-N-Rar");
			goto CLEANUP;
		}

		//Get all info about later versions
		CString sText = "You are currently running v" + theApp.GetMyVersion();
		sText += "\n";
		sText += "The latest version is v" + sVer;
		sText += "\n\n";

		MSXML::IXMLDOMNodeListPtr nlVersions = domVersions->selectNodes( "/Versions/Version");	
		for (long l=0; l<nlVersions->length; l++)
		{
			eVer = nlVersions->Getitem(l);
			sVer = static_cast<char *>(_bstr_t(eVer->getAttribute("number")));
			if (sVer == theApp.GetMyVersion())
				break;
			sText += "****************************************************\nVersion " + sVer;
			sText += ": \n";
			sText += static_cast<char *>(eVer->text);
			sText += "\n\n";
		}
		ctlDetails.ReplaceSel(sText);
	}

CLEANUP:
	Resize();
	EndWaitCursor();
}

void NewVersionDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	if (!IsWindowVisible())
		return;
	Resize();
}

void NewVersionDlg::Resize()
{
	CRect rectForm, rectButton;
    
	GetClientRect(&rectForm);
	GetDlgItem(IDOK)->GetClientRect(rectButton);
	GetDlgItem(IDOK)->MoveWindow(rectForm.Width() - rectButton.Width() - 2, rectForm.Height() - rectButton.Height() - 2, rectButton.Width(), rectButton.Height(), TRUE);	
	ctlDetails.MoveWindow(0, 0, rectForm.Width(), rectForm.Height() - rectButton.Height() - 5, TRUE);
}
