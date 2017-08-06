
#pragma once

namespace Vixen
{
	namespace MFC
	{
	/*!
	 * @class Doc
	 * @brief Subclass of MFC CDocument that supports serialization of
	 * scene manager binary files (.vix) by overriding OnNewDocument.
	 *
	 * If you want the normal MFC open dialogs to understand
	 * the .vix extension and load scene manager scene files, you need
	 * to derive from MFC::Doc instead of CDocument.
	 *
	 * @ingroup vixenmfc
	 * @see App View
	 */
		class Doc : public CDocument
		{
		protected: // create from serialization only
			Doc();
			DECLARE_DYNCREATE(Doc)

		public:
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(Doc)
			public:
			virtual BOOL OnNewDocument();
			virtual BOOL OnSaveDocument(LPCTSTR fname);
			virtual void Serialize(CArchive& ar);
			//}}AFX_VIRTUAL

		// Implementation
		public:
		#ifdef _DEBUG
			virtual void AssertValid() const;
			virtual void Dump(CDumpContext& dc) const;
		#endif

		protected:
		// Data
			CString		m_FileName;		// full path of current file
			CString		m_FileRoot;		// name of file (without directory & extension)
			CString		m_ModelName;

		// Generated message map functions
		protected:
			//{{AFX_MSG(Doc)
			afx_msg void OnFileOpen();
			afx_msg void OnFileSaveAs();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};
	}	// end MFC
}	// end Vixen