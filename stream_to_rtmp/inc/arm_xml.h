/*******************************************************************************
 *           file:    arm_xml.h                                                *
 *          Title:    header file for archiving xml wrapper                    *
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
 *                    [x] Windows  [ ] CYGWIN  [ ] Unixware                    *
 *                                                                             *
 * Change History:                                                             *
 * 1.0.0.0  Feb 25, 2010 5:21:01 PM  (liangyj)  Created                        *
 *                                                                             *
 ******************************************************************************/


#ifndef ARM_XML_H_
#define ARM_XML_H_

char * parse_child(ezxml_t xml, const char *name);
char * parse_attrs(ezxml_t xml, const char *include);
ezxml_t ezxml_set_interattr(ezxml_t xml, const char *name, int value);
ezxml_t set_xml_node(ezxml_t xml, const char *name, const char *values);


#endif /* ARM_XML_H_ */
