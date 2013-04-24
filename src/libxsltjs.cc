#include <libxml/HTMLparser.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <cstring>
#include <node.h>
#include "libxsltjs.h"

using namespace v8;

void WeakCallbackXmlTemplate(Persistent<Value> value, void *data)
{
	HandleScope scope;
	
	xmlDocPtr doc = (xmlDocPtr)data;
	xmlFreeDoc(doc);
	
	value.Dispose();
	value.Clear();
}

void WeakCallbackXslTemplate(Persistent<Value> value, void *data)
{
	HandleScope scope;
	
	xsltStylesheetPtr doc = (xsltStylesheetPtr)data;
	xsltFreeStylesheet(doc);
	
	value.Dispose();
	value.Clear();
}
  
Handle<Value> ReadXmlString(const Arguments &args)
{
	HandleScope scope;
	
	// Make sure we have been passed a single string as an argument
	if (args.Length() != 1)
	{
		ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
		return scope.Close(Undefined());
	}
	
	// Convert string to Utf8
	String::Utf8Value str(args[0]);

	// Parse the string using libxml
	xmlDocPtr doc = xmlReadMemory(*str, str.length(), NULL, "UTF-8", 0);
	if (!doc)
	{
		ThrowException(Exception::TypeError(String::New("Failed to parse XML")));
		return scope.Close(Undefined());
	}
	
	// Create a template for returning the C++ object to Javascript
	Handle<ObjectTemplate> xmlTemplate = ObjectTemplate::New();
	xmlTemplate->SetInternalFieldCount(1);
	// Create a persistent Javascript instance of this object
	// Add our cleanup function to free the C++ object when the Javascript one is returned to the heap
	Persistent<Object> weakHandle = Persistent<Object>::New(xmlTemplate->NewInstance());
	weakHandle->SetInternalField(0, External::New(doc));
	weakHandle.MakeWeak((void *)doc, WeakCallbackXmlTemplate);
	
	// Attach the resulting object to the scope and return
	return scope.Close(weakHandle);
}

Handle<Value> ReadXsltString(const Arguments &args)
{
	HandleScope scope;
	
	// Make sure we have been passed a single string as an argument
	if (args.Length() != 1)
	{
		ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
		return scope.Close(Undefined());
	}
	
	// Convert string to Utf8
	String::Utf8Value str(args[0]);

	// Parse the string using libxml
	xmlDocPtr doc = xmlReadMemory(*str, str.length(), NULL, "UTF-8", 0);
	if (!doc)
	{
		ThrowException(Exception::TypeError(String::New("Failed to parse XML")));
		return scope.Close(Undefined());
	}

	xsltStylesheetPtr stylesheet = xsltParseStylesheetDoc(doc);
	if (!stylesheet)
	{
		ThrowException(Exception::TypeError(String::New("Failed to parse stylesheet")));
		return scope.Close(Undefined());
	}
    
	// Create a template for returning the C++ object to Javascript
	Handle<ObjectTemplate> xslTemplate = ObjectTemplate::New();
	xslTemplate->SetInternalFieldCount(1);
	// Create a persistent Javascript instance of this object
	// Add our cleanup function to free the C++ object when the Javascript one is returned to the heap
	Persistent<Object> weakHandle = Persistent<Object>::New(xslTemplate->NewInstance());
	weakHandle->SetInternalField(0, External::New(stylesheet));
	weakHandle.MakeWeak((void *)stylesheet, WeakCallbackXslTemplate);
	
		
	// Attach the resulting object to the scope and return
	return scope.Close(weakHandle);
}

Handle<Value> Transform(const Arguments &args)
{
	HandleScope scope;
	
	// Make sure we have been passed a single string as an argument
	if (args.Length() != 3)
	{
		ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
		return scope.Close(Undefined());
	}
	
	// Make sure the third argument is a valid array
	if (!args[(2)]->IsArray())
	{
		ThrowException(Exception::TypeError(String::New("Third argument not a valid array")));
		return scope.Close(Undefined());
	}
	
	// Cast array and check length has an even number of params
        Handle<Array> array = Handle<Array>::Cast(args[2]);
	uint32_t arrayLen = array->Length();
	if ((arrayLen % 2) != 0)
	{
		ThrowException(Exception::TypeError(String::New("Array contains an odd number of parameters")));
		return scope.Close(Undefined());
	}
        
        // Cast the Javascript variables to objects ready to retrieve the pointers
	Local<Object> objStylesheet = args[0]->ToObject();
	Local<Object> objDocument = args[1]->ToObject();
        
	xsltStylesheetPtr stylesheet = (xsltStylesheetPtr) (Local<External>::Cast(objStylesheet->GetInternalField(0))->Value());
    	xmlDocPtr document = (xmlDocPtr) (Local<External>::Cast(objDocument->GetInternalField(0))->Value());

	// Build the parameter list
	char **params = (char **)malloc(sizeof(char *) *(arrayLen + 1));
	if (!params)
	{
		ThrowException(Exception::TypeError(String::New("Failed to allocate memory")));
		return scope.Close(Undefined());
	}
	memset(params, 0, sizeof(char *) * (array->Length() + 1));

	// Create the param string
	for (unsigned int i = 0; i < array->Length(); i++)
	{
		Local<String> param = array->Get(Integer::New(i))->ToString();
		params[i] = (char *)malloc(sizeof(char) * (param->Length() + 1));
		if (!params[i])
		{
		    	ThrowException(Exception::TypeError(String::New("Failed to allocate memory")));
			return scope.Close(Undefined());
		}
		param->WriteAscii(params[i]);
	}

	// Apply the style sheet to the document
        xmlDocPtr result = xsltApplyStylesheet(stylesheet, document, (const char **)params);

        // Free the params memory
        for (unsigned int i = 0; i < array->Length(); i++)
		free(params[i]);
	free(params);

        if (!result)
        {
		ThrowException(Exception::TypeError(String::New("Failed to apply stylesheet")));
		return scope.Close(Undefined());
        }

        xmlChar *doc_ptr;
        int doc_len;

        // convert the resulting transformed stylesheet to a string
        xsltSaveResultToString(&doc_ptr, &doc_len, result, stylesheet);

        // Free the result memory
	xmlFreeDoc(result);

        if (!doc_ptr)
        {
        	// Free the document memory since we aren't returning anything now
        	free(doc_ptr);
        	return String::Empty();
        }
	return scope.Close(String::New((const char *)doc_ptr, doc_len));
}

void Initialize(Handle<Object> exports)
{
	exports->Set(String::NewSymbol("readXmlString"), FunctionTemplate::New(ReadXmlString)->GetFunction());
	exports->Set(String::NewSymbol("readXsltString"), FunctionTemplate::New(ReadXsltString)->GetFunction());
	exports->Set(String::NewSymbol("transform"), FunctionTemplate::New(Transform)->GetFunction());
      	
	/*BIND("readXmlFile", readXmlFile);
	BIND("readHtmlString", readHtmlString);
	BIND("readHtmlFile", readHtmlFile);
	BIND("readXsltFile", readXsltFile);*/
}

NODE_MODULE(nodexsltjs, Initialize)
