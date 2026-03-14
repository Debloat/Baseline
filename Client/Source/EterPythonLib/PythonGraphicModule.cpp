#include "StdAfx.h"
#include "../EterLib/Camera.h"
#include "../EterLib/TextBar.h"

#include <shlobj.h>

PyObject* grpCreateTextBar(PyObject* poSelf, PyObject* poArgs)
{
    int iWidth;

    if (!PyTuple_GetInteger(poArgs, 0, &iWidth))
    {
        return Py_BuildException();
    }

    int iHeight;

    if (!PyTuple_GetInteger(poArgs, 1, &iHeight))
    {
        return Py_BuildException();
    }

    CTextBar * pTextBar = new CTextBar(12, false);

    if (!pTextBar->Create(NULL, iWidth, iHeight))
    {
        delete pTextBar;
        return Py_BuildValue("i", NULL);
    }

    return Py_BuildValue("i", pTextBar);
}

PyObject* grpCreateBigTextBar(PyObject* poSelf, PyObject* poArgs)
{
    int iWidth;

    if (!PyTuple_GetInteger(poArgs, 0, &iWidth))
    {
        return Py_BuildException();
    }

    int iHeight;

    if (!PyTuple_GetInteger(poArgs, 1, &iHeight))
    {
        return Py_BuildException();
    }

    int iFontSize;

    if (!PyTuple_GetInteger(poArgs, 2, &iFontSize))
    {
        return Py_BuildException();
    }

    CTextBar * pTextBar = new CTextBar(iFontSize, true);

    if (!pTextBar->Create(NULL, iWidth, iHeight))
    {
        delete pTextBar;
        return Py_BuildValue("i", NULL);
    }

    return Py_BuildValue("i", pTextBar);
}

PyObject* grpDestroyTextBar(PyObject* poSelf, PyObject* poArgs)
{
    int iHandle;

    if (!PyTuple_GetInteger(poArgs, 0, &iHandle))
    {
        return Py_BuildException();
    }

    CTextBar * pTextBar = (CTextBar*)iHandle;
    delete pTextBar;

    return Py_BuildNone();
}

PyObject* grpRenderTextBar(PyObject* poSelf, PyObject* poArgs)
{
    int iHandle;

    if (!PyTuple_GetInteger(poArgs, 0, &iHandle))
    {
        return Py_BuildException();
    }

    int ix;

    if (!PyTuple_GetInteger(poArgs, 1, &ix))
    {
        return Py_BuildException();
    }

    int iy;

    if (!PyTuple_GetInteger(poArgs, 2, &iy))
    {
        return Py_BuildException();
    }

    CTextBar * pTextBar = (CTextBar*)iHandle;

    if (pTextBar)
    {
        pTextBar->Render(ix, iy);
    }

    return Py_BuildNone();
}

PyObject* grpTextBarSetTextColor(PyObject* poSelf, PyObject* poArgs)
{
    int iHandle;

    if (!PyTuple_GetInteger(poArgs, 0, &iHandle))
    {
        return Py_BuildException();
    }

    int r;

    if (!PyTuple_GetInteger(poArgs, 1, &r))
    {
        return Py_BuildException();
    }

    int g;

    if (!PyTuple_GetInteger(poArgs, 2, &g))
    {
        return Py_BuildException();
    }

    int b;

    if (!PyTuple_GetInteger(poArgs, 3, &b))
    {
        return Py_BuildException();
    }

    CTextBar * pTextBar = (CTextBar*)iHandle;

    if (pTextBar)
    {
        pTextBar->SetTextColor(r, g, b);
    }

    return Py_BuildNone();
}

PyObject* grpTextBarGetTextExtent(PyObject* poSelf, PyObject* poArgs)
{
    int iHandle;

    if (!PyTuple_GetInteger(poArgs, 0, &iHandle))
    {
        return Py_BuildException();
    }

    char* szText;

    if (!PyTuple_GetString(poArgs, 1, &szText))
    {
        return Py_BuildException();
    }

    SIZE size = {0, 0};
    CTextBar * pTextBar = (CTextBar*)iHandle;

    if (pTextBar)
    {
        pTextBar->GetTextExtent(szText, &size);
    }

    return Py_BuildValue("ii", size.cx, size.cy);
}

PyObject* grpTextBarTextOut(PyObject* poSelf, PyObject* poArgs)
{
    int iHandle;

    if (!PyTuple_GetInteger(poArgs, 0, &iHandle))
    {
        return Py_BuildException();
    }

    int ix;

    if (!PyTuple_GetInteger(poArgs, 1, &ix))
    {
        return Py_BuildException();
    }

    int iy;

    if (!PyTuple_GetInteger(poArgs, 2, &iy))
    {
        return Py_BuildException();
    }

    char* szText;

    if (!PyTuple_GetString(poArgs, 3, &szText))
    {
        return Py_BuildException();
    }

    CTextBar * pTextBar = (CTextBar*)iHandle;

    if (pTextBar)
    {
        pTextBar->TextOut(ix, iy, szText);
    }

    return Py_BuildNone();
}

PyObject* grpClearTextBar(PyObject* poSelf, PyObject* poArgs)
{
    int iHandle;

    if (!PyTuple_GetInteger(poArgs, 0, &iHandle))
    {
        return Py_BuildException();
    }

    CTextBar * pTextBar = (CTextBar*)iHandle;

    if (pTextBar)
    {
        pTextBar->ClearBar();
    }

    return Py_BuildNone();
}

PyObject* grpSetTextBarClipRect(PyObject* poSelf, PyObject* poArgs)
{
    int iHandle;

    if (!PyTuple_GetInteger(poArgs, 0, &iHandle))
    {
        return Py_BuildException();
    }

    int isx;

    if (!PyTuple_GetInteger(poArgs, 1, &isx))
    {
        return Py_BuildException();
    }

    int isy;

    if (!PyTuple_GetInteger(poArgs, 2, &isy))
    {
        return Py_BuildException();
    }

    int iex;

    if (!PyTuple_GetInteger(poArgs, 3, &iex))
    {
        return Py_BuildException();
    }

    int iey;

    if (!PyTuple_GetInteger(poArgs, 4, &iey))
    {
        return Py_BuildException();
    }

    CTextBar * pTextBar = (CTextBar*)iHandle;

    if (pTextBar)
    {
        RECT rect;
        rect.left = isx;
        rect.top = isy;
        rect.right = iex;
        rect.bottom = iey;

        pTextBar->SetClipRect(rect);
    }

    return Py_BuildNone();
}

PyObject* grpCulling(PyObject* poSelf, PyObject* poArgs)
{
    CCullingManager::Instance().Process();
    return Py_BuildNone();
}

PyObject* grpInitScreenEffect(PyObject* poSelf, PyObject* poArgs)
{
    CPythonGraphic::Instance().InitScreenEffect();
    return Py_BuildNone();
}

PyObject* grpClearDepthBuffer(PyObject* poSelf, PyObject* poArgs)
{
    CPythonGraphic::Instance().ClearDepthBuffer();
    return Py_BuildNone();
}

PyObject* grpPushState(PyObject* poSelf, PyObject* poArgs)
{
    CPythonGraphic::Instance().PushState();
    return Py_BuildNone();
}

PyObject* grpPopState(PyObject* poSelf, PyObject* poArgs)
{
    CPythonGraphic::Instance().PopState();
    return Py_BuildNone();
}

PyObject* grpTranslate(PyObject* poSelf, PyObject* poArgs)
{
    float x;

    if (!PyTuple_GetFloat(poArgs, 0, &x))
    {
        return Py_BuildException("grp.Translate:Wrong X");
    }

    float y;

    if (!PyTuple_GetFloat(poArgs, 1, &y))
    {
        return Py_BuildException("grp.Translate:Wrong Y");
    }

    float z;

    if (!PyTuple_GetFloat(poArgs, 2, &z))
    {
        return Py_BuildException("grp.Translate:Wrong Z");
    }

    CPythonGraphic::Instance().Translate(x, y, z);

    return Py_BuildNone();
}

PyObject* grpRotate(PyObject* poSelf, PyObject* poArgs)
{
    float Degree;

    if (!PyTuple_GetFloat(poArgs, 0, &Degree))
    {
        return Py_BuildException();
    }

    float x;

    if (!PyTuple_GetFloat(poArgs, 1, &x))
    {
        return Py_BuildException();
    }

    float y;

    if (!PyTuple_GetFloat(poArgs, 2, &y))
    {
        return Py_BuildException();
    }

    float z;

    if (!PyTuple_GetFloat(poArgs, 3, &z))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().Rotate(Degree, x, y, z);

    return Py_BuildNone();
}

PyObject* grpSetPerspective(PyObject* poSelf, PyObject* poArgs)
{
    float fov;

    if (!PyTuple_GetFloat(poArgs, 0, &fov))
    {
        return Py_BuildException();
    }

    float aspect;

    if (!PyTuple_GetFloat(poArgs, 1, &aspect))
    {
        return Py_BuildException();
    }

    float zNear;

    if (!PyTuple_GetFloat(poArgs, 2, &zNear))
    {
        return Py_BuildException();
    }

    float zFar;

    if (!PyTuple_GetFloat(poArgs, 3, &zFar))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().SetPerspective(fov, aspect, zNear, zFar);
    return Py_BuildNone();
}

PyObject* grpGenerateColor(PyObject* poSelf, PyObject* poArgs)
{
    float r;

    if (!PyTuple_GetFloat(poArgs, 0, &r))
    {
        return Py_BuildException();
    }

    float g;

    if (!PyTuple_GetFloat(poArgs, 1, &g))
    {
        return Py_BuildException();
    }

    float b;

    if (!PyTuple_GetFloat(poArgs, 2, &b))
    {
        return Py_BuildException();
    }

    float a;

    if (!PyTuple_GetFloat(poArgs, 3, &a))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("l", CPythonGraphic::Instance().GenerateColor(r, g, b, a));
}

PyObject* grpSetColor(PyObject* poSelf, PyObject* poArgs)
{
    int color;

    if (!PyTuple_GetInteger(poArgs, 0, &color))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().SetDiffuseColor(DWORD(color));
    return Py_BuildNone();
}

PyObject* grpSetAlpha(PyObject* poSelf, PyObject* poArgs)
{
    float Alpha;

    if (!PyTuple_GetFloat(poArgs, 0, &Alpha))
    {
        return Py_BuildException();
    }

    //CPythonGraphic::Instance().SetAlpha(Alpha);
    return Py_BuildNone();
}

PyObject* grpGetCursorPosition3d(PyObject * poSelf, PyObject * poArgs)
{
    float x, y, z;
    CPythonGraphic::Instance().GetCursorPosition(&x, &y, &z);
    return Py_BuildValue("(fff)", x, y, z);
}

PyObject* grpSetCursorPosition(PyObject * poSelf, PyObject * poArgs)
{
    int ix;

    if (!PyTuple_GetInteger(poArgs, 0, &ix))
    {
        return Py_BuildException();
    }

    int iy;

    if (!PyTuple_GetInteger(poArgs, 1, &iy))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().SetCursorPosition(ix, iy);
    return Py_BuildNone();
}

PyObject* grpRenderLine(PyObject* poSelf, PyObject* poArgs)
{
    int x;

    if (!PyTuple_GetInteger(poArgs, 0, &x))
    {
        return Py_BuildException();
    }

    int y;

    if (!PyTuple_GetInteger(poArgs, 1, &y))
    {
        return Py_BuildException();
    }

    int width;

    if (!PyTuple_GetInteger(poArgs, 2, &width))
    {
        return Py_BuildException();
    }

    int height;

    if (!PyTuple_GetInteger(poArgs, 3, &height))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().RenderLine2d((float) x, (float) y, (float) x + width, (float) y + height);
    return Py_BuildNone();
}

PyObject* grpRenderRoundBox(PyObject* poSelf, PyObject* poArgs)
{
    int x;

    if (!PyTuple_GetInteger(poArgs, 0, &x))
    {
        return Py_BuildException();
    }

    int y;

    if (!PyTuple_GetInteger(poArgs, 1, &y))
    {
        return Py_BuildException();
    }

    int width;

    if (!PyTuple_GetInteger(poArgs, 2, &width))
    {
        return Py_BuildException();
    }

    int height;

    if (!PyTuple_GetInteger(poArgs, 3, &height))
    {
        return Py_BuildException();
    }

    float fx = float(x);
    float fy = float(y);
    float fWidth = float(width);
    float fHeight = float(height);

    CPythonGraphic::Instance().RenderLine2d(fx + 2.0f, fy, fx + 2.0f + (fWidth - 3.0f), fy);
    CPythonGraphic::Instance().RenderLine2d(fx + 2.0f, fy + fHeight, fx + 2.0f + (fWidth - 3.0f), fy + fHeight);
    CPythonGraphic::Instance().RenderLine2d(fx, fy + 2.0f, fx, fy + 2.0f + fHeight - 4.0f);
    CPythonGraphic::Instance().RenderLine2d(fx + fWidth, fy + 1.0f, fx + fWidth, fy + 1.0f + fHeight - 3.0f);
    CPythonGraphic::Instance().RenderLine2d(fx, fy + 2.0f, fx + 2.0f, fy);
    CPythonGraphic::Instance().RenderLine2d(fx, fy + fHeight - 2.0f, fx + 2.0f, fy + fHeight);
    CPythonGraphic::Instance().RenderLine2d(fx + fWidth - 2.0f, fy, fx + fWidth, fy + 2.0f);
    CPythonGraphic::Instance().RenderLine2d(fx + fWidth - 2.0f, fy + fHeight, fx + fWidth, fy + fHeight - 2.0f);
    return Py_BuildNone();
}

PyObject* grpRenderBox(PyObject* poSelf, PyObject* poArgs)
{
    int x;

    if (!PyTuple_GetInteger(poArgs, 0, &x))
    {
        return Py_BuildException();
    }

    int y;

    if (!PyTuple_GetInteger(poArgs, 1, &y))
    {
        return Py_BuildException();
    }

    int width;

    if (!PyTuple_GetInteger(poArgs, 2, &width))
    {
        return Py_BuildException();
    }

    int height;

    if (!PyTuple_GetInteger(poArgs, 3, &height))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().RenderBox2d((float) x, (float) y, (float) x + width, (float) y + height);
    return Py_BuildNone();
}

PyObject* grpRenderBar(PyObject* poSelf, PyObject* poArgs)
{
    int x;

    if (!PyTuple_GetInteger(poArgs, 0, &x))
    {
        return Py_BuildException();
    }

    int y;

    if (!PyTuple_GetInteger(poArgs, 1, &y))
    {
        return Py_BuildException();
    }

    int width;

    if (!PyTuple_GetInteger(poArgs, 2, &width))
    {
        return Py_BuildException();
    }

    int height;

    if (!PyTuple_GetInteger(poArgs, 3, &height))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().RenderBar2d((float) x, (float) y, (float) x + width, (float) y + height);
    return Py_BuildNone();
}

PyObject* grpRenderGradationBar(PyObject* poSelf, PyObject* poArgs)
{
    int x;

    if (!PyTuple_GetInteger(poArgs, 0, &x))
    {
        return Py_BadArgument();
    }

    int y;

    if (!PyTuple_GetInteger(poArgs, 1, &y))
    {
        return Py_BadArgument();
    }

    int width;

    if (!PyTuple_GetInteger(poArgs, 2, &width))
    {
        return Py_BadArgument();
    }

    int height;

    if (!PyTuple_GetInteger(poArgs, 3, &height))
    {
        return Py_BadArgument();
    }

    int iStartColor;

    if (!PyTuple_GetInteger(poArgs, 4, &iStartColor))
    {
        return Py_BadArgument();
    }

    int iEndColor;

    if (!PyTuple_GetInteger(poArgs, 5, &iEndColor))
    {
        return Py_BadArgument();
    }

    CPythonGraphic::Instance().RenderGradationBar2d((float)x, (float)y, (float)x + width, (float)y + height, iStartColor, iEndColor);
    return Py_BuildNone();
}

PyObject* grpSetClearColor(PyObject * poSelf, PyObject * poArgs)
{
    float fr;

    if (!PyTuple_GetFloat(poArgs, 0, &fr))
    {
        return Py_BuildException();
    }

    float fg;

    if (!PyTuple_GetFloat(poArgs, 1, &fg))
    {
        return Py_BuildException();
    }

    float fb;

    if (!PyTuple_GetFloat(poArgs, 2, &fb))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().SetClearColor(fr, fg, fb, 1.0f);
    return Py_BuildNone();
}

PyObject* grpSetDiffuseColor(PyObject * poSelf, PyObject * poArgs)
{
    float r;

    if (!PyTuple_GetFloat(poArgs, 0, &r))
    {
        return Py_BuildException();
    }

    float g;

    if (!PyTuple_GetFloat(poArgs, 1, &g))
    {
        return Py_BuildException();
    }

    float b;

    if (!PyTuple_GetFloat(poArgs, 2, &b))
    {
        return Py_BuildException();
    }

    float a;

    if (!PyTuple_GetFloat(poArgs, 3, &a))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().SetDiffuseColor(r, g, b, a);
    return Py_BuildNone();
}

PyObject* grpGetAvailableMemory(PyObject * poSelf, PyObject * poArgs)
{
    int iValue = CPythonGraphic::Instance().GetAvailableMemory();
    return Py_BuildValue("i", iValue);
}

// SCREENSHOT_CWDSAVE
PyObject* grpSaveScreenShotToPath(PyObject * poSelf, PyObject * poArgs)
{
    char* szBasePath;

    if (!PyTuple_GetString(poArgs, 0, &szBasePath))
    {
        return Py_BuildException();
    }

    struct tm * tmNow;

    time_t ct;

    ct = time(0);
    tmNow = localtime(&ct);

    char szPath[MAX_PATH + 256];
    snprintf(szPath, sizeof(szPath), "%s%02d%02d_%02d%02d%02d.jpg",
             szBasePath,
             tmNow->tm_mon + 1,
             tmNow->tm_mday,
             tmNow->tm_hour,
             tmNow->tm_min,
             tmNow->tm_sec);

    BOOL bResult = CPythonGraphic::Instance().SaveScreenShot(szPath);
    return Py_BuildValue("(is)", bResult, szPath);
}

// END_OF_SCREENSHOT_CWDSAVE

PyObject* grpSaveScreenShot(PyObject * poSelf, PyObject * poArgs)
{
    struct tm * tmNow;
    time_t ct;

    ct = time(0);
    tmNow = localtime(&ct);

    char szPath[MAX_PATH + 256];
    SHGetSpecialFolderPath(NULL, szPath, CSIDL_PERSONAL, TRUE);
    //GetTempPath();
    strcat(szPath, "\\METIN2\\");

    if (-1 == _access(szPath, 0))
        if (!CreateDirectory(szPath, NULL))
        {
            TraceError("Failed to create directory [%s]\n", szPath);
            return Py_BuildValue("(is)", FALSE, "");
        }

    sprintf(szPath + strlen(szPath), "%02d%02d_%02d%02d%02d.jpg",
            tmNow->tm_mon + 1,
            tmNow->tm_mday,
            tmNow->tm_hour,
            tmNow->tm_min,
            tmNow->tm_sec);

    BOOL bResult = CPythonGraphic::Instance().SaveScreenShot(szPath);
    return Py_BuildValue("(is)", bResult, szPath);
}

PyObject* grpSetInterfaceRenderState(PyObject * poSelf, PyObject * poArgs)
{
    CPythonGraphic::Instance().SetInterfaceRenderState();
    return Py_BuildNone();
}

PyObject* grpSetViewport(PyObject * poSelf, PyObject * poArgs)
{
    float fx;

    if (!PyTuple_GetFloat(poArgs, 0, &fx))
    {
        return Py_BuildException();
    }

    float fy;

    if (!PyTuple_GetFloat(poArgs, 1, &fy))
    {
        return Py_BuildException();
    }

    float fWidth;

    if (!PyTuple_GetFloat(poArgs, 2, &fWidth))
    {
        return Py_BuildException();
    }

    float fHeight;

    if (!PyTuple_GetFloat(poArgs, 3, &fHeight))
    {
        return Py_BuildException();
    }

    UINT uWidth;
    UINT uHeight;
    CPythonGraphic::Instance().GetBackBufferSize(&uWidth, &uHeight);
    CPythonGraphic::Instance().SetViewport(fx * uWidth, fy * uHeight, fWidth * uWidth, fHeight * uHeight);
    return Py_BuildNone();
}

PyObject* grpRestoreViewport(PyObject * poSelf, PyObject * poArgs)
{
    CPythonGraphic::Instance().RestoreViewport();
    return Py_BuildNone();
}

PyObject* grpGetCameraPosition(PyObject * poSelf, PyObject * poArgs)
{
    D3DXVECTOR3 v3Eye = CCameraManager::Instance().GetCurrentCamera()->GetEye();
    return Py_BuildValue("fff", v3Eye.x, v3Eye.y, v3Eye.z);
}

PyObject* grpGetTargetPosition(PyObject * poSelf, PyObject * poArgs)
{
    D3DXVECTOR3 v3Target = CCameraManager::Instance().GetCurrentCamera()->GetTarget();
    return Py_BuildValue("fff", v3Target.x, v3Target.y, v3Target.z);
}

void initgrp()
{
    static PyMethodDef s_methods[] =
    {
        { "InitScreenEffect",			grpInitScreenEffect,			METH_VARARGS },
        { "Culling",					grpCulling,						METH_VARARGS },
        { "ClearDepthBuffer",			grpClearDepthBuffer,			METH_VARARGS },
        { "GenerateColor",				grpGenerateColor,				METH_VARARGS },
        { "PopState",					grpPopState,					METH_VARARGS },
        { "PushState",					grpPushState,					METH_VARARGS },
        { "Translate",					grpTranslate,					METH_VARARGS },
        { "Rotate",						grpRotate,						METH_VARARGS },
        { "SetPerspective",				grpSetPerspective,				METH_VARARGS },
        { "SetColor",					grpSetColor,					METH_VARARGS },
        { "SetAlpha",					grpSetAlpha,					METH_VARARGS },
        { "SetDiffuseColor",			grpSetDiffuseColor,				METH_VARARGS },
        { "SetClearColor",				grpSetClearColor,				METH_VARARGS },
        { "GetCursorPosition3d",		grpGetCursorPosition3d,			METH_VARARGS },
        { "SetCursorPosition",			grpSetCursorPosition,			METH_VARARGS },
        { "RenderLine",					grpRenderLine,					METH_VARARGS },
        { "RenderBox",					grpRenderBox,					METH_VARARGS },
        { "RenderRoundBox",				grpRenderRoundBox,				METH_VARARGS },
        { "RenderBar",					grpRenderBar,					METH_VARARGS },
        { "RenderGradationBar",			grpRenderGradationBar,			METH_VARARGS },
        { "GetAvailableMemory",			grpGetAvailableMemory,			METH_VARARGS },
        { "SaveScreenShot",				grpSaveScreenShot,				METH_VARARGS },
        { "SaveScreenShotToPath",		grpSaveScreenShotToPath,		METH_VARARGS },
        { "SetInterfaceRenderState",	grpSetInterfaceRenderState,		METH_VARARGS },
        { "SetViewport",				grpSetViewport,					METH_VARARGS },
        { "RestoreViewport",			grpRestoreViewport,				METH_VARARGS },
        { "GetCameraPosition",			grpGetCameraPosition,			METH_VARARGS },
        { "GetTargetPosition",			grpGetTargetPosition,			METH_VARARGS },

        { "CreateTextBar",				grpCreateTextBar,				METH_VARARGS },
        { "CreateBigTextBar",			grpCreateBigTextBar,			METH_VARARGS },
        { "DestroyTextBar",				grpDestroyTextBar,				METH_VARARGS },
        { "RenderTextBar",				grpRenderTextBar,				METH_VARARGS },
        { "TextBarTextOut",				grpTextBarTextOut,				METH_VARARGS },
        { "TextBarSetTextColor",		grpTextBarSetTextColor,			METH_VARARGS },
        { "TextBarGetTextExtent",		grpTextBarGetTextExtent,		METH_VARARGS },
        { "ClearTextBar",				grpClearTextBar,				METH_VARARGS },
        { "SetTextBarClipRect",			grpSetTextBarClipRect,			METH_VARARGS },

        { NULL,							NULL,							NULL		},
    };

    Py_InitModule("grp", s_methods);
}
