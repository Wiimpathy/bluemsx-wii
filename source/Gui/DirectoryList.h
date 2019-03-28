#ifndef _H_DIRLIST
#define _H_DIRLIST

#include "expat.h"

typedef enum {
  DIR_CONTAINER_ROOT,
  DIR_CONTAINER_DIRLIST,
  DIR_CONTAINER_DIR,
} DIR_CONTAINER;

class DirElement
{
public:
    DirElement();
    virtual ~DirElement();

    void SetName(const char *str);
    void SetDirectory(const char *str);
    char *GetName();
    char *GetDirectory();
    DirElement *next;
private:
    char *name;
    char *directory;
};

class DirList
{
public:
    DirList();
    virtual ~DirList();

    int Load(const char *filename);
    void Clear(void);
    int GetNumberOfDirs(void);
    DirElement* GetDir(int index);

    static void XMLCALL startElement(void *userData, const char *name, const char **atts);
    static void XMLCALL endElement(void *userData, const char *name);
    static void XMLCALL dataHandler(void *userData, const XML_Char *s, int len);
private:
    int elements;
    char *receiving_string;
    DirElement *current_element;
    DirElement *first_element;
    DirElement *last_element;
    DIR_CONTAINER current_container;
};

#endif

