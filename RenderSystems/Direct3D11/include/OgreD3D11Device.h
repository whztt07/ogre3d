/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2015 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef __D3D11DEVICE_H__
#define __D3D11DEVICE_H__


#include "OgreD3D11Prerequisites.h"

namespace Ogre
{
    class D3D11Device
    {
    public:
        static const char* sDebugLevelUndefined;
        static const char* sDebugLevelNone;
        static const char* sDebugLevelMessage;
        static const char* sDebugLevelInfo;
        static const char* sDebugLevelWarning;
        static const char* sDebugLevelError;
        static const char* sDebugLevelCorruption;

        enum eDebugErrorLevel
        {
            DEL_UNDEFINED,
            DEL_NO_EXCEPTION,
            DEL_CORRUPTION,
            DEL_ERROR,
            DEL_WARNING,
            DEL_INFO,
            DEL_MESSAGE
        };

    private:
        ID3D11DeviceN*          mD3D11Device;
        ID3D11DeviceContextN*   mImmediateContext;
        ID3D11ClassLinkage*     mClassLinkage;
        ID3D11InfoQueue*        mInfoQueue;
        bool                    mInfoQueueDirty;
        eDebugErrorLevel        mStorageErrorLevel;
        eDebugErrorLevel        mExceptionsErrorLevel;

#if OGRE_D3D11_PROFILING
        ID3DUserDefinedAnnotation* mPerf;
#endif

        /*************************************************************************/
        // Private abstract copy constructor and copy assignment operator intentionally not implemented,
        // copying instances of D3D11Device is not allowed in D3D11Device class.
        D3D11Device(const D3D11Device& device);
        const D3D11Device& operator=(D3D11Device& device);
        /*************************************************************************/

        void clearStoredErrorMessages() const;
        void refreshInfoQueue();
        bool _getErrorsFromQueue() const;
        void markInfoQueueDirty();
        typedef std::vector<D3D11_MESSAGE_SEVERITY> SevrityList;
        void generateSeverity(const eDebugErrorLevel severityLevel, SevrityList& severityList);
        eDebugErrorLevel getDebugErrorLevelFromSeverity(const D3D11_MESSAGE_SEVERITY severity) const;
        D3D11Device::eDebugErrorLevel parseErrorLevel(const String& errorLevel);

    public:
        D3D11Device();
        ~D3D11Device();
        void ReleaseAll();
        void TransferOwnership(ID3D11DeviceN* device);
        bool isNull()                                { return mD3D11Device == 0; }
        ID3D11DeviceN* get()                         { return mD3D11Device; }
        ID3D11DeviceContextN* GetImmediateContext()  { return mImmediateContext; }
        ID3D11ClassLinkage* GetClassLinkage()        { return mClassLinkage; }
#if OGRE_D3D11_PROFILING
        ID3DUserDefinedAnnotation* GetProfiler()     { return mPerf; }
#endif
        
        ID3D11DeviceN* operator->() const{return mD3D11Device;}

        String getErrorDescription(const HRESULT hr = NO_ERROR) const;

        bool isError() const { return (DEL_NO_EXCEPTION == mExceptionsErrorLevel) ? false : _getErrorsFromQueue(); }

        //Exception error level
        void setExceptionsErrorLevel(const eDebugErrorLevel errorLevel);
        void setExceptionsErrorLevel(const String& errorLevel);
        const eDebugErrorLevel getExceptionsErrorLevel();
        const String getExceptionErrorLevelAsString();

        //storage (Debug output error level)
        void setStorageErrorLevel(const eDebugErrorLevel errorLevel);
        void setStorageErrorLevel(const String& errorLevel);
        const eDebugErrorLevel getStorageErrorLevel();
        const String getStorageErrorLevelAsString();

        String errorLevelToString(const D3D11Device::eDebugErrorLevel errorLevel);
    };
}
#endif
