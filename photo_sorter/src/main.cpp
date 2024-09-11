#include <iostream>
#include <string>
#include <shlobj.h>
#include <filesystem>

namespace fs = std::filesystem;

bool func(char (* path)[MAX_PATH])
{
    /*
        Modified but i'll still credit
        Post : https://stackoverflow.com/questions/12034943/win32-select-directory-dialog-from-c-c
        User : https://stackoverflow.com/users/2990789/sparkyrobinson
    */

    BROWSEINFOA bi = { 0 };
    bi.lpszTitle  = ("Selectionne le dossier avec les photos a trier\nSi tu a ouvert ca par accident, appuye sur Annuler");
    bi.ulFlags    = BIF_RETURNONLYFSDIRS;// | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST pidl = SHBrowseForFolderA ( &bi );

    if ( pidl == 0 ) return true;

    //get the name of the folder and put it in path
    SHGetPathFromIDListA ( pidl, *path );

    //free memory used
    IMalloc * imalloc = 0;
    if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
    {
        imalloc->Free ( pidl );
        imalloc->Release ( );
    }

    return false;
}


int main(int argc, char const *argv[])
{
    char folderPathBuffer[MAX_PATH];

    if (func(&folderPathBuffer))
        return 0;
    
    std::cout << folderPathBuffer << '\n';

    std::string path = folderPathBuffer;
    path += "\\";

    std::string temp;

    for (const auto & entry : fs::directory_iterator(folderPathBuffer))
    {
        if (entry.is_directory()) continue;
        if (entry.path().filename().string().substr(0, 4) != "PXL_") continue;
        temp = entry.path().filename().string().substr(4, 6).insert(4, " - ");
        // std::cout << entry.path().filename().string().substr(4, 6).insert(4, "-") << '\n';
        system(("md \"" + path + temp + " - Vie de famille\"").c_str());
        system(("move /-Y \"" + entry.path().string() + "\" \"" + path + temp + " - Vie de famille\"").c_str());
    }

    
    
    {   // previous stuff that i'm leaving here
    // char path[MAX_PATH];

    // LPBROWSEINFOA lpbi;
    //     lpbi->hwndOwner = NULL; // Owner window handle
    //     lpbi->lpfn      = NULL; // Callback function
    //     lpbi->lParam    = NULL; // Callback function argument
    //     lpbi->lpszTitle = "Select the folder with the images to be sorted"; // Window title
    //     lpbi->pidlRoot  = NULL; // Root from which to start browsing
    //     lpbi->pszDisplayName    = &path[0]; // Folder display name buffer pointer
    //     lpbi->ulFlags   = 0;
    
    // PIDLIST_ABSOLUTE



    // char filename[ MAX_PATH ];


    // OPENFILENAMEA ofn;
    //     ZeroMemory( &filename, sizeof( filename ) );
    //     ZeroMemory( &ofn,      sizeof( ofn ) );
    //     ofn.lStructSize  = sizeof( ofn );
    //     ofn.hwndOwner    = NULL;  // If you have a window to center over, put its HANDLE here
    //     ofn.lpstrFilter  = "Any File\0*.*\0";
    //     ofn.lpstrFile    = filename;
    //     ofn.lpstrInitialDir = "%homePath%/Pictures";
    //     ofn.nMaxFile     = MAX_PATH;
    //     ofn.lpstrTitle   = "Select an image to open :";
    //     ofn.Flags        = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

        

    // if (GetOpenFileNameA( &ofn ))
    // {
    //     std::cout << filename << '\n';
    // }
    // else
    // {
    //     // std::cout << "Error while trying to open file.\n";
    // }
    }
    return 0;
}