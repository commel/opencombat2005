// SAXContentHandlerImpl.cpp: implementation of the SAXContentHandlerImpl class.
//
//////////////////////////////////////////////////////////////////////
#import <msxml4.dll> raw_interfaces_only 
using namespace MSXML2;
#include "SAXContentHandlerImpl.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


SAXContentHandlerImpl::SAXContentHandlerImpl()
{
}

SAXContentHandlerImpl::~SAXContentHandlerImpl()
{
}



HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::putDocumentLocator( 
            /* [in] */ ISAXLocator __RPC_FAR *pLocator
            )
{
	UNREFERENCED_PARAMETER(pLocator);
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::startDocument()
{
    return S_OK;
}
        

        
HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::endDocument( void)
{
    return S_OK;
}
        
        
HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::startPrefixMapping( 
            /* [in] */ unsigned short __RPC_FAR *pwchPrefix,
            /* [in] */ int cchPrefix,
            /* [in] */ unsigned short __RPC_FAR *pwchUri,
            /* [in] */ int cchUri)
{
	UNREFERENCED_PARAMETER(pwchPrefix);
	UNREFERENCED_PARAMETER(cchPrefix);
	UNREFERENCED_PARAMETER(pwchUri);
	UNREFERENCED_PARAMETER(cchUri);
    return S_OK;
}
        
        
HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::endPrefixMapping( 
            /* [in] */ unsigned short __RPC_FAR *pwchPrefix,
            /* [in] */ int cchPrefix)
{
	UNREFERENCED_PARAMETER(pwchPrefix);
	UNREFERENCED_PARAMETER(cchPrefix);
    return S_OK;
}
        

        
HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::startElement( 
            /* [in] */ unsigned short __RPC_FAR *pwchNamespaceUri,
            /* [in] */ int cchNamespaceUri,
            /* [in] */ unsigned short __RPC_FAR *pwchLocalName,
            /* [in] */ int cchLocalName,
            /* [in] */ unsigned short __RPC_FAR *pwchRawName,
            /* [in] */ int cchRawName,
            /* [in] */ ISAXAttributes __RPC_FAR *pAttributes)
{
	UNREFERENCED_PARAMETER(pwchNamespaceUri);
	UNREFERENCED_PARAMETER(cchNamespaceUri);
	UNREFERENCED_PARAMETER(pwchLocalName);
	UNREFERENCED_PARAMETER(cchLocalName);
	UNREFERENCED_PARAMETER(pwchRawName);
	UNREFERENCED_PARAMETER(cchRawName);
	UNREFERENCED_PARAMETER(pAttributes);
    return S_OK;
}
        
       
HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::endElement( 
            /* [in] */ unsigned short __RPC_FAR *pwchNamespaceUri,
            /* [in] */ int cchNamespaceUri,
            /* [in] */ unsigned short __RPC_FAR *pwchLocalName,
            /* [in] */ int cchLocalName,
            /* [in] */ unsigned short __RPC_FAR *pwchRawName,
            /* [in] */ int cchRawName)
{
	UNREFERENCED_PARAMETER(pwchNamespaceUri);
	UNREFERENCED_PARAMETER(cchNamespaceUri);
	UNREFERENCED_PARAMETER(pwchLocalName);
	UNREFERENCED_PARAMETER(cchLocalName);
	UNREFERENCED_PARAMETER(pwchRawName);
	UNREFERENCED_PARAMETER(cchRawName);
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::characters( 
            /* [in] */ unsigned short __RPC_FAR *pwchChars,
            /* [in] */ int cchChars)
{
	UNREFERENCED_PARAMETER(pwchChars);
	UNREFERENCED_PARAMETER(cchChars);
    return S_OK;
}
        

HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::ignorableWhitespace( 
            /* [in] */ unsigned short __RPC_FAR *pwchChars,
            /* [in] */ int cchChars)
{
	UNREFERENCED_PARAMETER(pwchChars);
	UNREFERENCED_PARAMETER(cchChars);
	return S_OK;
}
        

HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::processingInstruction( 
            /* [in] */ unsigned short __RPC_FAR *pwchTarget,
            /* [in] */ int cchTarget,
            /* [in] */ unsigned short __RPC_FAR *pwchData,
            /* [in] */ int cchData)
{
	UNREFERENCED_PARAMETER(pwchTarget);
	UNREFERENCED_PARAMETER(cchTarget);
	UNREFERENCED_PARAMETER(pwchData);
	UNREFERENCED_PARAMETER(cchData);

    return S_OK;
}
        
        
HRESULT STDMETHODCALLTYPE SAXContentHandlerImpl::skippedEntity( 
            /* [in] */ unsigned short __RPC_FAR *pwchVal,
            /* [in] */ int cchVal)
{
	UNREFERENCED_PARAMETER(pwchVal);
	UNREFERENCED_PARAMETER(cchVal);

    return S_OK;
}


long __stdcall SAXContentHandlerImpl::QueryInterface(const struct _GUID &riid,void ** ppvObject)
{
    // hack-hack-hack!
	UNREFERENCED_PARAMETER(ppvObject);
	UNREFERENCED_PARAMETER(riid);

	return 0;
}

unsigned long __stdcall SAXContentHandlerImpl::AddRef()
{
    // hack-hack-hack!
    return 0;
}

unsigned long __stdcall SAXContentHandlerImpl::Release()
{
    // hack-hack-hack!
    return 0;
}

