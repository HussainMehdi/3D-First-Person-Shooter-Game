/****************************************
* zoidcom_typehelper.h
* template stuff
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2007 by Joerg Rueppel. See documentation for copyright and licensing details.
*****************************************/

#ifndef _ZOIDCOM_TYPEHELPER_H
#define _ZOIDCOM_TYPEHELPER_H

//////////////////////////////////////////////////////////////////////////
// template helper
//////////////////////////////////////////////////////////////////////////

template <typename T> struct ZCom_TypeHelper
{
  typedef T  value_type;
  typedef T* pointer;
};
template <typename T> struct ZCom_TypeHelper<T*>
{
  typedef T  value_type;
  typedef T* pointer;
};

#endif
