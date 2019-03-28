#include <stdio.h>
#include <string.h>
#include <fat.h>
#include "DirectoryList.h"

/*************************************************
  Directory Element
 *************************************************/

DirElement::DirElement()
{
    name = NULL;
    directory = NULL;
    next = NULL;
}

DirElement::~DirElement()
{
    if( name ) free(name);
    if( directory ) free(directory);
}

void DirElement::SetName(const char *str)
{
    if( name ) free(name);
    name = strdup(str);
}

void DirElement::SetDirectory(const char *str)
{
    if( directory ) free(directory);
    directory = strdup(str);
}

char* DirElement::GetName()
{
    return name;
}

char* DirElement::GetDirectory()
{
    return directory;
}


/*************************************************
  Directory List
 *************************************************/

void XMLCALL DirList::startElement(void *userData, const char *name, const char **atts)
{
    DirList *my = (DirList *)userData;

    switch( my->current_container ) {
    case DIR_CONTAINER_ROOT:
        if( strcmp("DirectoryList", name)==0 ) {
            my->current_container = DIR_CONTAINER_DIRLIST;
        }
        break;
    case DIR_CONTAINER_DIRLIST:
        if( strcmp("Item", name)==0 &&
            atts[0] != NULL && strcmp("Name", atts[0])==0 &&
            atts[2] != NULL && strcmp("Directory", atts[2])==0 ) {
            my->current_container = DIR_CONTAINER_DIR;
            if( my->current_element != NULL ) {
                // throw away unfinished elements
                delete my->current_element;
            }
            my->current_element = new DirElement();
            my->current_element->SetName(atts[1]);
            my->current_element->SetDirectory(atts[3]);
        }
        break;
    case DIR_CONTAINER_DIR:
        break;
    }
}

void XMLCALL DirList::endElement(void *userData, const char *name)
{
    DirList *my = (DirList *)userData;

    switch( my->current_container ) {
    case DIR_CONTAINER_ROOT:
        break;
    case DIR_CONTAINER_DIRLIST:
        my->current_container = DIR_CONTAINER_ROOT;
        break;
    case DIR_CONTAINER_DIR:
        my->current_container = DIR_CONTAINER_DIRLIST;
        // if first element, set it to first_element
        if( my->first_element == NULL ){
            my->first_element = my->current_element;
        }
        // append to linked-list
        if( my->last_element != NULL ){
            my->last_element->next = my->current_element;
        }
        my->elements++;
        // set as last element
        my->last_element = my->current_element;
        // current element again free to use
        my->current_element = NULL;
        break;
    }
}

void XMLCALL DirList::dataHandler(void *userData, const XML_Char *s, int len)
{
}

int DirList::GetNumberOfDirs(void)
{
    return elements;
}

DirElement* DirList::GetDir(int index)
{
    DirElement *p = first_element;
    while(p && index--) {
        p = p->next;
    }
    return p;
}

int DirList::Load(const char *filename)
{
    char buf[1024];
    FILE *f;

    // Clear first
    Clear();

    // Parse XML
    current_container = DIR_CONTAINER_ROOT;
    f = fopen(filename, "rb");
    if( f == NULL ) {
        return 0;
    }else{
        XML_Parser parser = XML_ParserCreate(NULL);
        int done;
        XML_SetUserData(parser, this);
        XML_SetElementHandler(parser, startElement, endElement);
        XML_SetCharacterDataHandler(parser, dataHandler);
        do {
            unsigned len = (int)fread(buf, 1, sizeof(buf), f);
            done = len < sizeof(buf);
            if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {
              fprintf(stderr,
                      "%s at line %lu\n",
                      XML_ErrorString(XML_GetErrorCode(parser)),
                      XML_GetCurrentLineNumber(parser));
              break;
            }
        } while (!done);
        XML_ParserFree(parser);
        fclose(f);
    }
    return elements;
}

void DirList::Clear(void)
{
    // Free list
    elements = 0;
    DirElement *p = first_element;
    while( p ) {
        DirElement *n = p->next;
        delete p;
        p = n;
    }
    first_element = NULL;
    last_element = NULL;
    // Free element in process
    if( current_element != NULL ) {
        delete current_element;
        current_element = NULL;
    }
    // Free strings
    if( receiving_string != NULL ) {
        free(receiving_string);
        receiving_string = NULL;
    }
}

DirList::DirList()
{
    // Init member variables
    elements = 0;
    current_element = NULL;
    first_element = NULL;
    last_element = NULL;
    receiving_string = NULL;
}

DirList::~DirList()
{
    Clear();
}

