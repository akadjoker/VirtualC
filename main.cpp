
#include <cstdio>
#include <string>
#include <vector>
#include <iostream>  // cout
#include <assert.h>  // assert()
#include <string.h>  // strstr()
#include <sys/time.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdarg.h>
#include <angelscript.h>
#include "scriptstdstring.h"
#include "scriptbuilder.h"
#include "scriptmath.h"
#include "scriptmathcomplex.h"
#include "scripthandle.h"
#include "scriptfile.h"
#include "scriptfilesystem.h"
#include "scriptarray.h"
#include "scriptany.h"
#include "weakref.h"
#include "scriptdictionary.h"
#include "datetime.h"
#include "scripthelper.h"
#include <raylib.h>
//#include "script_call.h"
using namespace std;


#ifdef _DEBUG
#define VERIFY(x) assert((x) >= 0)
#else
#define VERIFY(x) x
#endif

static int g_argc;
static char* g_argv0;

static int g_retval = 0;



typedef unsigned int DWORD;

// Linux doesn't have timeGetTime(), this essentially does the same
// thing, except this is milliseconds since Epoch (Jan 1st 1970) instead
// of system start. It will work the same though...
DWORD timeGetTime()
{
	timeval time;
	gettimeofday(&time, NULL);
	return time.tv_sec*1000 + time.tv_usec/1000;
}


int	get_time_now(void)
{
	int				t;
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	t = (int)(tv.tv_sec + tv.tv_usec / 1e6);
	return (t);
}

uint64_t	get_timer_value(void)
{
	struct timeval	tv;
	gettimeofday(&tv, NULL);
	return ((uint64_t)tv.tv_sec * (uint64_t)1000000 + (uint64_t)tv.tv_usec);
}

float	get_timer_float(void)
{
	struct timeval	tv;
	gettimeofday(&tv, NULL);
	return ((float)tv.tv_sec * (float)1.0f + (float)tv.tv_usec);
}


float	get_timer_now(void)
{
	struct timeval	tv;
	gettimeofday(&tv, NULL);
	return ( (float)tv.tv_usec / 1000000.f);
}



int time_offset;

int	get_time()
{
	return ((int)(get_timer_value() - time_offset) / 1000);
}

/*void ScriptPrint(const std::string &str)
{
	printf("%s\n", str.c_str());
}*/

void PrintString(string &str)
{
	cout << str << endl;
}

// Function implementation with generic script interface
void PrintString_Generic(asIScriptGeneric *gen)
{
	string *str = (string*)gen->GetArgAddress(0);
	cout << *str;
}

bool replace(std::string& str, const std::string& from, const std::string& to)
{
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

void as_printf(asIScriptGeneric *gen)
{

	void *ref = gen->GetArgAddress(0);
	int typeId = gen->GetArgTypeId(0);

	string format = *static_cast<string*>(ref);

	for (int i = 1; i < 16; i++)
	{
		ref = gen->GetArgAddress(i);
		typeId = gen->GetArgTypeId(i);

		switch (typeId)
		{
			case 67108876: //string?
			{
				string local = *static_cast<string*>(ref);
				replace(format, "%s", local);
				break;
			}
			case 2:
			{
				char local = *static_cast<char*>(ref);
				replace(format, "%d", to_string(local));
				break;
			}
			case 3:
			{
				short local = *static_cast<short*>(ref);
				replace(format, "%d", to_string(local));
				break;
			}
			case 4:
			{
				int local = *static_cast<int*>(ref);
				replace(format, "%d", to_string(local));
				break;
			}
			case 5:
			{
				long long local = *static_cast<long long*>(ref);
				replace(format, "%d", to_string(local));
				break;
			}
			case 6:
			{
				unsigned char local = *static_cast<unsigned char*>(ref);
				replace(format, "%d", to_string(local));
				break;
			}
			case 7:
			{
				unsigned short local = *static_cast<unsigned short*>(ref);
				replace(format, "%d", to_string(local));
				break;
			}
			case 8:
			{
				unsigned int local = *static_cast<unsigned int*>(ref);
				replace(format, "%d", to_string(local));
				break;
			}
			case 9:
			{
				unsigned long long local = *static_cast<unsigned long long*>(ref);
				replace(format, "%d", to_string(local));
				break;
			}
			case 10:
			{
				float local = *static_cast<float*>(ref);
				replace(format, "%f", to_string(local));
				break;
			}
			case 11:
			{
				double local = *static_cast<double*>(ref);
				replace(format, "%f", to_string(local));
				break;
			}
		}
	}

	cout << format << endl;
	return;
}

// Function wrapper is needed when native calling conventions are not supported




#ifdef _DEBUG
#define VERIFY(x) assert((x) >= 0)
#else
#define VERIFY(x) x
#endif



static void ScriptMessage(const asSMessageInfo* msg, void* param)
{
	const char* type = "";
	switch (msg->type) {
		case asMSGTYPE_ERROR: type = "ERROR"; break;
		case asMSGTYPE_WARNING: type = "WARN"; break;
		case asMSGTYPE_INFORMATION: type = "INFO"; break;
	}
	printf("| %s | Line %d | %s | %s\n", msg->section, msg->row, type, msg->message);
}

static std::string ScriptStringReplace(std::string* self, const std::string &search, const std::string &replace)
{
	std::string ret = *self;
	size_t startPos = 0;
	while ((startPos = ret.find(search, startPos)) != std::string::npos) {
		ret.replace(startPos, search.length(), replace);
		startPos += replace.length();
	}
	return ret;
}

// Linux does have a getch() function in the curses library, but it doesn't
// work like it does on DOS. So this does the same thing, with out the need
// of the curses library.
int getch()
{
	struct termios oldt, newt;
	int ch;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );

	ch = getchar();

	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}

// Function prototypes
int  RunApplication();
void ConfigureEngine(asIScriptEngine *engine);
int  CompileScript(asIScriptEngine *engine);
void LineCallback(asIScriptContext *ctx, DWORD *timeOut);


asIScriptContext* g_ctx = nullptr;


void MessageCallback(const asSMessageInfo *msg, void *param)
{
	const char *type = "ERR ";
	if( msg->type == asMSGTYPE_WARNING )
		type = "WARN";
	else if( msg->type == asMSGTYPE_INFORMATION )
		type = "INFO";

	printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}


void timeGetTime_Generic(asIScriptGeneric *gen)
{
	gen->SetReturnDWord(get_timer_value());
}

void timeGetTimePass_Generic(asIScriptGeneric *gen)
{
	gen->SetReturnDWord(get_time());
}

void timeGetTimeFloat_Generic(asIScriptGeneric *gen)
{
	gen->SetReturnFloat(get_timer_float());
}

void timeGetTimeNow_Generic(asIScriptGeneric *gen)
{
	gen->SetReturnFloat(get_timer_now());
}

void Assert_generic(asIScriptGeneric *gen)
{
	int b = (int)gen->GetArgWord(0);
	assert(b);
}

void sleep_generic(asIScriptGeneric *gen)
{
	int t = (int)gen->GetArgWord(0);
	#ifdef _WIN32
    Sleep(t);
    #else
    sleep(t);
    #endif

}


int RunApplication()
{
	int r;

	time_offset = get_timer_value();
	// Create the script engine
	asIScriptEngine *engine = asCreateScriptEngine();
	if( engine == 0 )
	{
		cout << "Failed to create script engine." << endl;
		return -1;
	}

	// The script compiler will write any compiler messages to the callback.
	engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);


	ConfigureEngine(engine);
	r = CompileScript(engine);
	if( r < 0 )
	{
		engine->Release();
		return -1;
	}
	g_ctx = engine->CreateContext();
	if( g_ctx == 0 )
	{
		cout << "Failed to create the context." << endl;
		engine->Release();
		return -1;
	}



	asIScriptFunction *func = engine->GetModule(0)->GetFunctionByDecl("void main()");
	if( func == 0 )
	{
		cout << "The function 'void main()' was not found." << endl;
		g_ctx->Release();
		engine->Release();
		return -1;
	}


	r = g_ctx->Prepare(func);
	if( r < 0 )
	{
		cout << "Failed to prepare the context." << endl;
		g_ctx->Release();
		engine->Release();
		return -1;
	}


	// Set the timeout before executing the function. Give the function 1 sec
	// to return before we'll abort it.
DWORD	timeOut = timeGetTime() + 10000;

	// Execute the function
	//cout << "Executing the script." << endl;
	//cout << "---" << endl;
	r = g_ctx->Execute();
	//cout << "---" << endl;
	if( r != asEXECUTION_FINISHED )
	{
		// The execution didn't finish as we had planned. Determine why.
		if( r == asEXECUTION_ABORTED )
			cout << "The script was aborted before it could finish. Probably it timed out." << endl;
		else if( r == asEXECUTION_EXCEPTION )
		{
			cout << "The script ended with an exception." << endl;

			// Write some information about the script exception
			asIScriptFunction *func = g_ctx->GetExceptionFunction();
			cout << "func: " << func->GetDeclaration() << endl;
			cout << "modl: " << func->GetModuleName() << endl;
			cout << "sect: " << func->GetScriptSectionName() << endl;
			cout << "line: " << g_ctx->GetExceptionLineNumber() << endl;
			cout << "desc: " << g_ctx->GetExceptionString() << endl;
		}
		else
			cout << "The script ended for some unforeseen reason (" << r << ")." << endl;
	}
	else
	{
		cout << "\n The script finished successfully." << endl;
	}



	g_ctx->Release();
	engine->ShutDownAndRelease();
	return 0;
}



void ConfigureEngine(asIScriptEngine *engine)
{
	int r;


	RegisterStdString(engine);
	RegisterScriptArray(engine,true);
	RegisterScriptDictionary(engine);
	RegisterScriptMath(engine);
	RegisterScriptMathComplex(engine);
	RegisterExceptionRoutines(engine);
	RegisterScriptDateTime(engine);
	RegisterScriptAny(engine);
	RegisterScriptFile(engine);
	RegisterScriptHandle(engine);
	RegisterStdStringUtils(engine);
	RegisterScriptWeakRef(engine);


/*

	 if (!strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY"))
	 {
    // Register the functions that the scripts will be allowed to use.
    // Note how the return code is validated with an assert(). This helps
    // us discover where a problem occurs, and doesn't pollute the code
    // with a lot of if's. If an error occurs in release mode it will
    // be caught when a script is being built, so it is not necessary
    // to do the verification here as well.
    r = engine->RegisterGlobalFunction("void print(string &in)",
                                       asFUNCTION(PrintString), asCALL_CDECL);
    assert(r >= 0);
    r = engine->RegisterGlobalFunction("uint GetSystemTime()",
                                       asFUNCTION(timeGetTime), asCALL_STDCALL);
    assert(r >= 0);

     r = engine->RegisterGlobalFunction("void print(string &in format, ?&in = null, ?&in = null, ?&in = null, ?&in = null, ?&in = null, ?&in = null)", asFUNCTION(asPrintf), asCALL_CDECL);
    assert(r >= 0);


    engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert_generic), asCALL_CDECL);
    assert(r >= 0);

    } else
    {
*/
    r = engine->RegisterGlobalFunction("void printf(string &in, ?&in var = 0, ?&in var2 = 0, ?&in var3 = 0, ?&in var4 = 0, ?&in var5 = 0, ?&in var6 = 0, ?&in var7 = 0, ?&in var8 = 0, ?&in var9 = 0, ?&in var10 = 0, ?&in var11 = 0, ?&in var12 = 0, ?&in var13 = 0, ?&in var14 = 0, ?&in var15 = 0)", asFUNCTION(as_printf), asCALL_GENERIC);
    assert(r >= 0);
    r = engine->RegisterGlobalFunction("void print(string &in, ?&in var = 0, ?&in var2 = 0, ?&in var3 = 0, ?&in var4 = 0, ?&in var5 = 0, ?&in var6 = 0, ?&in var7 = 0, ?&in var8 = 0, ?&in var9 = 0, ?&in var10 = 0, ?&in var11 = 0, ?&in var12 = 0, ?&in var13 = 0, ?&in var14 = 0, ?&in var15 = 0)", asFUNCTION(as_printf), asCALL_GENERIC);
    assert(r >= 0);
    r = engine->RegisterGlobalFunction("void trace(string &in, ?&in var = 0, ?&in var2 = 0, ?&in var3 = 0, ?&in var4 = 0, ?&in var5 = 0, ?&in var6 = 0, ?&in var7 = 0, ?&in var8 = 0, ?&in var9 = 0, ?&in var10 = 0, ?&in var11 = 0, ?&in var12 = 0, ?&in var13 = 0, ?&in var14 = 0, ?&in var15 = 0)", asFUNCTION(as_printf), asCALL_GENERIC);
    assert(r >= 0);
    r = engine->RegisterGlobalFunction("int GetTime()",asFUNCTION(timeGetTime_Generic),asCALL_GENERIC);assert(r >= 0);
    r = engine->RegisterGlobalFunction("int GetTimePass()",asFUNCTION(timeGetTimePass_Generic),asCALL_GENERIC);assert(r >= 0);
    r = engine->RegisterGlobalFunction("float GetTimeNow()",asFUNCTION(timeGetTimeNow_Generic),asCALL_GENERIC);assert(r >= 0);
    r = engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert_generic), asCALL_GENERIC);assert(r >= 0);
    r = engine->RegisterGlobalFunction("void sleep(int)", asFUNCTION(sleep_generic), asCALL_GENERIC);assert(r >= 0);



   // r = engine->RegisterGlobalFunction("void print(string &in format, ?&in = null, ?&in = null, ?&in = null, ?&in = null, ?&in = null, ?&in = null)", asFUNCTION(asPrintf), asCALL_GENERIC);
    //assert(r >= 0);


  //}




    //r = engine->RegisterGlobalFunction("void printf(string &in,int i)", asFUNCTION(PrintString), asCALL_CDECL); assert( r >= 0 );
    //r = engine->RegisterGlobalFunction("uint GetSystemTime()", asFUNCTION(timeGetTime), asCALL_STDCALL); assert( r >= 0 );


}

int CompileScript(asIScriptEngine *engine)
{
	int r;

	// The builder is a helper class that will load the script file,
	// search for #include directives, and load any included files as
	// well.
	CScriptBuilder builder;

	// Build the script. If there are any compiler messages they will
	// be written to the message stream that we set right after creating the
	// script engine. If there are no errors, and no warnings, nothing will
	// be written to the stream.
	r = builder.StartNewModule(engine, 0);
	if( r < 0 )
	{
		cout << "Failed to start new module" << endl;
		return r;
	}
	r = builder.AddSectionFromFile("main.as");
	if( r < 0 )
	{
		cout << "Failed to add script file" << endl;
		return r;
	}
	r = builder.BuildModule();
	if( r < 0 )
	{
		cout << "Failed to build the module" << endl;
		return r;
	}

	// The engine doesn't keep a copy of the script sections after Build() has
	// returned. So if the script needs to be recompiled, then all the script
	// sections must be added again.

	// If we want to have several scripts executing at different times but
	// that have no direct relation with each other, then we can compile them
	// into separate script modules. Each module use their own namespace and
	// scope, so function names, and global variables will not conflict with
	// each other.

	return 0;
}

void LineCallback(asIScriptContext *ctx, DWORD *timeOut)
{
	// If the time out is reached we abort the script
	if( *timeOut < timeGetTime() )
		ctx->Abort();

	// It would also be possible to only suspend the script,
	// instead of aborting it. That would allow the application
	// to resume the execution where it left of at a later
	// time, by simply calling Execute() again.
}



int main(int argc, char **argv)
{
	RunApplication();
	//cout << endl << "Press any key to quit." << endl;
	//while(!getch());

	return 0;
}
