/*
 Copyright 1995-2004 ESRI

 All rights reserved under the copyright laws of the United States.

 You may freely redistribute and use this sample code, with or without modification.

 Disclaimer: THE SAMPLE CODE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED 
 WARRANTIES, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ESRI OR 
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 INTERRUPTION) SUSTAINED BY YOU OR A THIRD PARTY, HOWEVER CAUSED AND ON ANY 
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT ARISING IN ANY 
 WAY OUT OF THE USE OF THIS SAMPLE CODE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 SUCH DAMAGE.

 For additional information contact: Environmental Systems Research Institute, Inc.

 Attn: Contracts Dept.

 380 New York Street

 Redlands, California, U.S.A. 92373 

 Email: contracts@esri.com
*/
#ifndef __LICENSEUTLITIES_ESRICPP_h__
#define __LICENSEUTLITIES_ESRICPP_h__

#include <iostream>
#include "ArcSDK.h"

// Initialize the application and check out a license if needed.
bool InitializeApp(esriLicenseExtensionCode license = 
                   (esriLicenseExtensionCode)0);

// Attempt to initialize
bool InitAttemptWithoutExtension(esriLicenseProductCode product);
bool InitAttemptWithExtension(esriLicenseProductCode product,
                              esriLicenseExtensionCode extension);

// Shutdown the application and check in the license if needed.
HRESULT ShutdownApp(esriLicenseExtensionCode license = 
                    (esriLicenseExtensionCode)0);

#endif    // __LICENSEUTLITIES_ESRICPP_h__
