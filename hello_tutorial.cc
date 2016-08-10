/// Copyright (c) 2012 The Native Client Authors. All rights reserved.
/// Use of this source code is governed by a BSD-style license that can be
/// found in the LICENSE file.
///
/// @file hello_tutorial.cc
/// This example demonstrates loading, running and scripting a very simple NaCl
/// module.  To load the NaCl module, the browser first looks for the
/// CreateModule() factory method (at the end of this file).  It calls
/// CreateModule() once to load the module code from your .nexe.  After the
/// .nexe code is loaded, CreateModule() is not called again.
///
/// Once the .nexe code is loaded, the browser than calls the CreateInstance()
/// method on the object returned by CreateModule().  It calls CreateInstance()
/// each time it encounters an <embed> tag that references your NaCl module.
///
/// The browser can talk to your NaCl module via the postMessage() Javascript
/// function.  When you call postMessage() on your NaCl module from the browser,
/// this becomes a call to the HandleMessage() method of your pp::Instance
/// subclass.  You can send messages back to the browser by calling the
/// PostMessage() method on your pp::Instance.  Note that these two methods
/// (postMessage() in Javascript and PostMessage() in C++) are asynchronous.
/// This means they return immediately - there is no waiting for the message
/// to be handled.  This has implications in your program design, particularly
/// when mutating property values that are exposed to both the browser and the
/// NaCl module.

#include <cstdio>
#include <string>
#include <string.h>
#include <sstream>
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"

#include "ppapi/c/pp_stdint.h"
#include "ppapi/c/ppb_file_io.h"
#include "ppapi/cpp/file_io.h"
#include "ppapi/cpp/file_ref.h"
#include "ppapi/cpp/file_system.h"
#include "ppapi/utility/completion_callback_factory.h"

#define MAXBUFLEN 256

extern char wordlist[MAXBUFLEN];
typedef enum {
	LOAD,
	PREDICT,
	ADD
} dictState; 
dictState dState = LOAD;


char* GetPrediction(const char* word);
int loadWords(const char* buf, unsigned int len);
void ResetCurPtr();
extern char loadedwords[MAXBUFLEN];
void dumpDict();
void addWord(const char* word);
void updateList(const char *word);

namespace {
// The expected string sent by the browser.
const char* const kHelloString = "hello";
const char* const kDone = "Loading Done";
// The string sent back to the browser upon receipt of a message
// containing "hello".
const char* const kReplyString = "hello from NaCl";
static int64_t prevOffset=0;
static bool writeFile = false;
} // namespace

/// The Instance class.  One of these exists for each instance of your NaCl
/// module on the web page.  The browser will ask the Module object to create
/// a new Instance for each occurence of the <embed> tag that has these
/// attributes:
///     type="application/x-nacl"
///     src="hello_tutorial.nmf"
/// To communicate with the browser, you must override HandleMessage() for
/// receiving messages from the browser, and use PostMessage() to send messages
/// back to the browser.  Note that this interface is asynchronous.
class HelloTutorialInstance : public pp::Instance {
 public:
  /// The constructor creates the plugin-side instance.
  /// @param[in] instance the handle to the browser-side plugin instance.
  explicit HelloTutorialInstance(PP_Instance instance) : pp::Instance(instance),
        callback_factory_(this),
        file_system_(this, PP_FILESYSTEMTYPE_LOCALPERSISTENT),
        file_system_ready_(false) {
	}
  virtual ~HelloTutorialInstance() {}

  pp::CompletionCallbackFactory<HelloTutorialInstance> callback_factory_;
  pp::FileSystem file_system_;
  bool file_system_ready_;
  private:
  /// Struct to hold various info about a file operation. Our scheme in this
  /// example is to allocate this information on the heap so that is persists
  /// after the asynchronous call until the callback is called, and is
  /// therefore available to the main thread to complete the requested operation
  struct Request {
    pp::FileRef ref;
    pp::FileIO file;
    std::string file_contents;
    int64_t offset;
    PP_FileInfo info;
  };

  virtual bool Init_File() {
    pp::CompletionCallback callback = callback_factory_.NewCallback(
        &HelloTutorialInstance::FileSystemOpenCallback);
    int32_t rv = file_system_.Open(10*1024*1024, callback);
    if (rv != PP_OK_COMPLETIONPENDING) {
      callback.Run(rv);
      return false;
    }
 pp::Var var_reply = pp::Var(kReplyString);
 //PostMessage(var_reply);
    return true;
  }



  void FileSystemOpenCallback(int32_t result) {
    if (result != PP_OK) {
      ShowErrorMessage("File system open call failed", result);
      return;
    }

    file_system_ready_ = true;
    // Notify the user interface that we're ready
    //PostMessage(pp::Var("READY|"));
    Load("/samp.csv");
  }

  bool Save(const std::string& file_name, const std::string& file_contents) {
    if (!file_system_ready_) {
      ShowErrorMessage("File system is not open", PP_ERROR_FAILED);
      return false;
    }
    
    HelloTutorialInstance::Request* request = new HelloTutorialInstance::Request;
    request->ref = pp::FileRef(file_system_, file_name.c_str());
    request->file = pp::FileIO(this);
    request->offset = prevOffset;
    request->file_contents = file_contents;
    int32_t rv;

    pp::CompletionCallback callback = callback_factory_.NewCallback(
          &HelloTutorialInstance::SaveOpenCallback, request);
    if(writeFile == false)
    {
    	rv = request->file.Open(request->ref,
        	PP_FILEOPENFLAG_WRITE|PP_FILEOPENFLAG_CREATE,
	        callback);
	writeFile = true;
    }
    else
    {
    	rv = request->file.Open(request->ref,
        	PP_FILEOPENFLAG_WRITE,
	        callback);
    }

    // Handle cleanup in the callback if error
    if (rv != PP_OK_COMPLETIONPENDING) {
      callback.Run(rv);
      return false;
    }
    return true;
  }

  void SaveOpenCallback(int32_t result, HelloTutorialInstance::Request* request) {
    if (result != PP_OK) {
      ShowErrorMessage("File open for write failed", result);
      delete request;
      return;
    }

    // It is an error to write 0 bytes to the file, however,
    // upon opening we have truncated the file to length 0
    if (request->file_contents.length() == 0) {
      pp::CompletionCallback callback = callback_factory_.NewCallback(
          &HelloTutorialInstance::SaveFlushCallback, request);
      int32_t rv = request->file.Flush(callback);

      // Handle cleanup in the callback if error
      if (rv != PP_OK_COMPLETIONPENDING) {
        callback.Run(rv);
        return;
      }
    } else if (request->file_contents.length() <= INT32_MAX) {
      pp::CompletionCallback callback = callback_factory_.NewCallback(
          &HelloTutorialInstance::SaveWriteCallback, request);
        
      int32_t rv = request->file.Write(request->offset,
          request->file_contents.c_str(),
          request->file_contents.length(), callback);

      // Handle cleanup in the callback if error
      if (rv != PP_OK_COMPLETIONPENDING) {
        callback.Run(rv);
        return;
      }
    } else {
      ShowErrorMessage("File too big", PP_ERROR_FILETOOBIG);
      delete request;
      return;
    }
  }

  void SaveWriteCallback(int32_t bytes_written,
      HelloTutorialInstance::Request* request) {

    // Ensure the content length is something write() can handle
    assert(request->file_contents.length() <= INT32_MAX);

    request->offset += bytes_written;

    if (bytes_written == (int32_t)request->file_contents.length()){
      // All bytes have been written, flush the write buffer to complete
      pp::CompletionCallback callback = callback_factory_.NewCallback(
          &HelloTutorialInstance::SaveFlushCallback, request);
      int32_t rv = request->file.Flush(callback);

      // Handle cleanup in the callback if error
      if (rv != PP_OK_COMPLETIONPENDING) {
        callback.Run(rv);
        return;
      }
    } else {
      // If all the bytes haven't been written call write again with remainder
      pp::CompletionCallback callback = callback_factory_.NewCallback(
          &HelloTutorialInstance::SaveWriteCallback, request);
      int32_t rv = request->file.Write(request->offset,
          request->file_contents.c_str() + request->offset,
          request->file_contents.length() - request->offset, callback);

      // Handle cleanup in the callback if error
      if (rv != PP_OK_COMPLETIONPENDING) {
        callback.Run(rv);
        return;
      }
    }
  }

  void SaveFlushCallback(int32_t result, HelloTutorialInstance::Request* request) {
    if (result != PP_OK) {
      ShowErrorMessage("File fail to flush", result);
      delete request;
      return;
    }
    prevOffset = request->offset;
    delete request;
  }

  bool Load(const std::string& file_name) {
    if (!file_system_ready_) {
      ShowErrorMessage("File system is not open", PP_ERROR_FAILED);
      return false;
    }

    HelloTutorialInstance::Request* request = new HelloTutorialInstance::Request;
    request->ref = pp::FileRef(file_system_, file_name.c_str());
    request->file = pp::FileIO(this);
    //ShowStatusMessage(file_name.c_str());

    pp::CompletionCallback callback = callback_factory_.NewCallback(
          &HelloTutorialInstance::LoadOpenCallback, request);
    int32_t rv = request->file.Open(request->ref, PP_FILEOPENFLAG_READ,
        callback);

    // Handle cleanup in the callback if error
    if (rv != PP_OK_COMPLETIONPENDING) {
      callback.Run(rv);
      return false;
    }
    return true;
  }

  void LoadOpenCallback(int32_t result, HelloTutorialInstance::Request* request) {
    if (result == PP_ERROR_FILENOTFOUND) {
      ShowStatusMessage("File not found");
      delete request;
      return;
    } else if (result != PP_OK) {
      ShowErrorMessage("File open for read failed", result);
      delete request;
      return;
    }

    pp::CompletionCallback callback = callback_factory_.NewCallback(
          &HelloTutorialInstance::LoadQueryCallback, request);
    int32_t rv = request->file.Query(&request->info, callback);

    // Handle cleanup in the callback if error
    if (rv != PP_OK_COMPLETIONPENDING) {
      callback.Run(rv);
      return;
    }
  }
  void LoadQueryCallback(int32_t result, HelloTutorialInstance::Request* request) {
    if (result != PP_OK) {
      ShowErrorMessage("File query failed", result);
      delete request;
      return;
    }

    // FileIO.Read() can only handle int32 sizes
    if (request->info.size > INT32_MAX) {
      ShowErrorMessage("File too big", PP_ERROR_FILETOOBIG);
      delete request;
      return;
    }

    // Allocate a buffer to read the file into
    // Here we must allocate on the heap so FileIO::Read will write to this
    // one and only copy of file_contents
    request->file_contents.resize(request->info.size, '\0');
    request->offset = 0;

    pp::CompletionCallback callback = callback_factory_.NewCallback(
        &HelloTutorialInstance::LoadReadCallback, request);
    int32_t rv = request->file.Read(request->offset,
        &request->file_contents[request->offset],
        request->file_contents.length(), callback);

    // Handle cleanup in the callback if error
    if (rv != PP_OK_COMPLETIONPENDING) {
      callback.Run(rv);
      return;
    }
  }

  void LoadReadCallback(int32_t bytes_read, HelloTutorialInstance::Request* request) {
    // If bytes_read < 0 then it indicates the error code
    if (bytes_read < 0) {
      ShowErrorMessage("File read failed", bytes_read);
      delete request;
      return;
    }

    // Ensure the content length is something read() can handle
    assert(request->file_contents.length() <= INT32_MAX);

    request->offset += bytes_read;

    if (request->offset == request->file_contents.length() || bytes_read == 0) {
      // Done reading, send content to the user interface
      //PostMessage(pp::Var("DISP|" + request->file_contents));
      prevOffset = request->offset;
      loadWords(request->file_contents.c_str(),(uint32_t)request->file_contents.length());
      delete request;
    } else {
      // Some bytes remain to be read, call read again with remainder
      pp::CompletionCallback callback = callback_factory_.NewCallback(
            &HelloTutorialInstance::LoadReadCallback, request);
      int32_t rv = request->file.Read(request->offset,
          &request->file_contents[request->offset],
          request->file_contents.length() - request->offset, callback);

      // Handle cleanup in the callback if error
      if (rv != PP_OK_COMPLETIONPENDING) {
        callback.Run(rv);
        return;
      }
    }
  }
  void ShowErrorMessage(const std::string& message, int32_t result) {
    std::stringstream ss;
    ss << "ERR|" << message << " -- Error #: " << result;
    //PostMessage(pp::Var(ss.str()));
  }

  void ShowStatusMessage(const std::string& message) {
    std::stringstream ss;
    ss << "STAT|" << message;
    //PostMessage(pp::Var(ss.str()));
  }
  /// Handler for messages coming in from the browser via postMessage().  The
  /// @a var_message can contain anything: a JSON string; a string that encodes
  /// method names and arguments; etc.  For example, you could use
  /// JSON.stringify in the browser to create a message that contains a method
  /// name and some parameters, something like this:
  ///   var json_message = JSON.stringify({ "myMethod" : "3.14159" });
  ///   nacl_module.postMessage(json_message);
  /// On receipt of this message in @a var_message, you could parse the JSON to
  /// retrieve the method name, match it to a function call, and then call it
  /// with the parameter.
  /// @param[in] var_message The message posted by the browser.\\bgl11-netapp04a\wg-H\HNAV_TWC\Private\IP-Client\src_repository\NTP Scripts
  virtual void HandleMessage(const pp::Var& var_message) {
    // TODO(sdk_user): 1. Make this function handle the incoming message.
//if (!var_message.is_string())
  //  return;
  std::string message = var_message.AsString();
  int ret = 0;

  //const char* keys = message.c_str();
 if (message == kHelloString) {
	 pp::Var var_reply = pp::Var(kReplyString);
   	 PostMessage(var_reply);
	}
 if (message == kDone) {
	PostMessage("Dbg: State Changed");
	Init_File();
	dState = PREDICT;
  }
 else{

    if ( dState != PREDICT )
    {
	  pp::Var var_reply;
	  ret = loadWords(message.c_str(),message.length());
	  if(ret == -1)
	  {
	 	var_reply = pp::Var("Not loaded!!");
	  }
	  else
	  {
	 	var_reply = pp::Var("Yes its loaded!!");
	  }
	  //dumpDict();
    }
    else
    {
	  char tempStr[200] = {0};
	  char *temp = strstr(message.c_str(),"ADD");
	  if(temp != NULL)
	  {
		temp = strtok((char*) message.c_str()," ");
		dState = ADD;
		temp = strtok(NULL," ");
	  }
	  if(dState != ADD)
	  {
	  	temp = strstr(message.c_str(),"UPDATE");
		  if(temp != NULL)
		  {
			temp = strtok((char*) message.c_str()," ");
			temp = strtok(NULL," ");
			updateList(temp);
			return;
	  	}
	  }
	  if ( !strcmp(message.c_str()," "))
	  {
	  	ResetCurPtr();
		//PostMessage("Dbg: Reset");
	  }
	  else
	  {
		  if ( dState == PREDICT )
		  { 
	  	  	char* pred = GetPrediction(message.c_str());
		  	if (pred)
		    	{
		      		pp::Var var_reply = pp::Var(pred);	
			        PostMessage(var_reply);
	  	    	}
		    	else
		    	{
		    	}
		  }
		  else
		  {
			  addWord(temp);
			  strcpy(tempStr,temp);
			  strcat(tempStr,",");
			  dState = PREDICT;
			  Save("/samp.csv",tempStr);
			  PostMessage(pp::Var("WordAdded"));
		  }
	  }
    }	    
 }
  }
};

/// The Module class.  The browser calls the CreateInstance() method to create
/// an instance of your NaCl module on the web page.  The browser creates a new
/// instance for each <embed> tag with type="application/x-nacl".
class HelloTutorialModule : public pp::Module {
 public:
  HelloTutorialModule() : pp::Module() {
	  }
  virtual ~HelloTutorialModule() {}

  /// Create and return a HelloTutorialInstance object.
  /// @param[in] instance The browser-side instance.
  /// @return the plugin-side instance.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new HelloTutorialInstance(instance);
  }
};

namespace pp {
/// Factory function called by the browser when the module is first loaded.
/// The browser keeps a singleton of this module.  It calls the
/// CreateInstance() method on the object you return to make instances.  There
/// is one instance per <embed> tag on the page.  This is the main binding
/// point for your NaCl module with the browser.
Module* CreateModule() {
  return new HelloTutorialModule();
}
}  // namespace pp
