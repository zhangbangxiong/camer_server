/*******************************************************************************
 *           file:    arm_xml.c                                                *
 *          Title:    archiving xml wrapper routine                            *
 *    Description:                                                             *
 *                                                                             *
 *                                                                             *
 *     Applicable:                                                             *
 *      Copyright:    Copyright (c) 2002-2010 DSGdata Inc.                     *
 *                                                                             *
 * Security Level:    [ ] Confidential                                         *
 *                    [x] Open to DSGdata R&D project related employees        *
 *                    [ ] Open to DSGdata R&D employees                        *
 *                    [ ] Open to DSGdata technical employees                  *
 *                    [ ] Open to public                                       *
 *                                                                             *
 *      Tested On:    [ ] AIX  [ ] HP-UX  [ ] SunOS  [ ] OSF1  [x] Linux       *
 *                    [ ] Windows  [ ] CYGWIN  [ ] Unixware                    *
 *                                                                             *
 * Change History:                                                             *
 * 1.0.0.0  Feb 25, 2010 5:21:23 PM  (liangyj)  Created                        *
 *                                                                             *
 ******************************************************************************/


#include "stdio.h"
#include "time.h"
#include "pthread.h"
#include "ezxml.h"
#include "arm_xml.h"
#include "info.h"
#include <strings.h>
#include "config.h"

char * parse_child(ezxml_t xml, const char *name)
{
    ezxml_t childfd;
    childfd = ezxml_child(xml, name);
    if(childfd == NULL)
    {
        return NULL;
    }

    return (childfd->txt);
}

ezxml_t set_xml_node(ezxml_t xml, const char *name, const char *values)
{
     ezxml_t spxml;

     spxml = ezxml_add_child(xml, name, 0);
     if(values != NULL)
         ezxml_set_txt(spxml, values);

     return spxml;
}

ezxml_t ezxml_set_interattr(ezxml_t xml, const char *name, int value)
{
    char tmpbuf[255] = {0};
    
    sprintf(tmpbuf, "%d", value);
    printf("tmpbuf = %s\n",tmpbuf);

    ezxml_set_attr(xml, name,  tmpbuf);

    return xml;

}

char * parse_attrs(ezxml_t xml, const char *include)
{
    char    **ca  = NULL;
    char    *attr_n = NULL;

    if (NULL == xml)
        return 0;

    ca = xml->attr;
    while(ca && *ca)
    {
        if (include && 0 == strcasecmp(*ca, include))
        {
            ca += 2;
            break;
        }
        ca += 2;
    }

    if(*ca != NULL)
    {
        attr_n = (char *)malloc(NORMAL_BUFFER_SIZE);
        strcpy(attr_n, *ca);
    }

    return attr_n;
}



