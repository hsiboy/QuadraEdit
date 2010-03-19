


#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

USERES("quadedit.res");
USEFORM("main.cpp", MainForm);
USEFORM("about.cpp", FormAbout);
USEUNIT("midi.cpp");
USEFORM("error.cpp", FormError);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
   try
   {
      Application->Initialize();
      Application->Title = "QuadraEdit";
        Application->CreateForm(__classid(TMainForm), &MainForm);
        Application->CreateForm(__classid(TFormAbout), &FormAbout);
        Application->CreateForm(__classid(TFormError), &FormError);
        Application->Run();
   }
   catch (Exception &exception)
   {
      Application->ShowException(&exception);
   }
   return 0;
}





