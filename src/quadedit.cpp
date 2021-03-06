


#include <vcl.h>
#include <stdio.h>
#include "main.h"
#include "debug.h"
#pragma hdrstop

USERES("quadedit.res");
USEFORM("main.cpp", MainForm);
USEFORM("about.cpp", FormAbout);
USEFORM("error.cpp", FormError);
USEUNIT("midi.cpp");
USEFORM("debug.cpp", FormDebug);
USEFORM("device.cpp", FormDevice);
USEUNIT("quadgt.cpp");
USEUNIT("display.cpp");
USEFORM("swap.cpp", FormSwap);
USEFORM("copy.cpp", FormCopy);
USEFORM("compare.cpp", FormCompare);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
   try
   {
      Application->Initialize();
      Application->Title = "QuadraEdit";

        Application->CreateForm(__classid(TMainForm), &MainForm);
        Application->CreateForm(__classid(TFormDebug), &FormDebug);
        Application->CreateForm(__classid(TFormAbout), &FormAbout);
        Application->CreateForm(__classid(TFormError), &FormError);
        Application->CreateForm(__classid(TFormDevice), &FormDevice);
        Application->CreateForm(__classid(TFormSwap), &FormSwap);
        Application->CreateForm(__classid(TFormCopy), &FormCopy);
        Application->CreateForm(__classid(TFormCompare), &FormCompare);
        FormDebug->Show();
        MainForm->Init();

        Application->Run();
   }
   catch (Exception &exception)
   {
      Application->ShowException(&exception);
   }
   return 0;
}






