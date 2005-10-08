// SAXContentHandlerImpl.h: interface for the SAXContentHandlerImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SAXCONTENTHANDLERIMPL_H__517D91C0_0BCF_474D_A77C_10EE27231233__INCLUDED_)
#define AFX_SAXCONTENTHANDLERIMPL_H__517D91C0_0BCF_474D_A77C_10EE27231233__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class SAXContentHandlerImpl : public ISAXContentHandler  
{
public:
    SAXContentHandlerImpl();
    virtual ~SAXContentHandlerImpl();

        // This must be correctly implemented, if your handler must be a COM Object (in this example it does not)
        long __stdcall QueryInterface(const struct _GUID &,void ** );
        unsigned long __stdcall AddRef(void);
        unsigned long __stdcall Release(void);

        virtual HRESULT STDMETHODCALLTYPE putDocumentLocator( 
            /* [in] */ ISAXLocator __RPC_FAR *pLocator);
        
        virtual HRESULT STDMETHODCALLTYPE startDocument( void);
        
        virtual HRESULT STDMETHODCALLTYPE endDocument( void);
        
        virtual HRESULT STDMETHODCALLTYPE startPrefixMapping( 
            /* [in] */ unsigned short __RPC_FAR *pwchPrefix,
            /* [in] */ int cchPrefix,
            /* [in] */ unsigned short __RPC_FAR *pwchUri,
            /* [in] */ int cchUri);
        
        virtual HRESULT STDMETHODCALLTYPE endPrefixMapping( 
            /* [in] */ unsigned short __RPC_FAR *pwchPrefix,
            /* [in] */ int cchPrefix);
        
        virtual HRESULT STDMETHODCALLTYPE startElement( 
            /* [in] */ unsigned short __RPC_FAR *pwchNamespaceUri,
            /* [in] */ int cchNamespaceUri,
            /* [in] */ unsigned short __RPC_FAR *pwchLocalName,
            /* [in] */ int cchLocalName,
            /* [in] */ unsigned short __RPC_FAR *pwchRawName,
            /* [in] */ int cchRawName,
            /* [in] */ ISAXAttributes __RPC_FAR *pAttributes);
        
        virtual HRESULT STDMETHODCALLTYPE endElement( 
            /* [in] */ unsigned short __RPC_FAR *pwchNamespaceUri,
            /* [in] */ int cchNamespaceUri,
            /* [in] */ unsigned short __RPC_FAR *pwchLocalName,
            /* [in] */ int cchLocalName,
            /* [in] */ unsigned short __RPC_FAR *pwchRawName,
            /* [in] */ int cchRawName);
        
        virtual HRESULT STDMETHODCALLTYPE characters( 
            /* [in] */ unsigned short __RPC_FAR *pwchChars,
            /* [in] */ int cchChars);
        
        virtual HRESULT STDMETHODCALLTYPE ignorableWhitespace( 
            /* [in] */ unsigned short __RPC_FAR *pwchChars,
            /* [in] */ int cchChars);
        
        virtual HRESULT STDMETHODCALLTYPE processingInstruction( 
            /* [in] */ unsigned short __RPC_FAR *pwchTarget,
            /* [in] */ int cchTarget,
            /* [in] */ unsigned short __RPC_FAR *pwchData,
            /* [in] */ int cchData);
        
        virtual HRESULT STDMETHODCALLTYPE skippedEntity( 
            /* [in] */ unsigned short __RPC_FAR *pwchName,
            /* [in] */ int cchName);

	static float ToFloat(unsigned short *str, int count)
	{
		UNREFERENCED_PARAMETER(count);
		return (float)_wtof((wchar_t *)str);
	}

	static int ToInt(unsigned short *str, int count)
	{
		UNREFERENCED_PARAMETER(count);
		return _wtoi((wchar_t *)str);
	}


};

#endif // !defined(AFX_SAXCONTENTHANDLERIMPL_H__517D91C0_0BCF_474D_A77C_10EE27231233__INCLUDED_)
