#include "LicenseUtilities.h"

// Initialize the application and check out a license if needed.
bool InitializeApp(esriLicenseExtensionCode license) 
{
  ::CoInitialize(NULL);
  IAoInitializePtr ipInit(CLSID_AoInitialize);

  if (license == 0)
  {
    // Try to init as engine, then engineGeoDB, then ArcView, 
    //    then ArcEditor, then ArcInfo 
    if (!InitAttemptWithoutExtension(esriLicenseProductCodeEngine))
      if (!InitAttemptWithoutExtension(esriLicenseProductCodeArcView))
        if (!InitAttemptWithoutExtension(esriLicenseProductCodeArcEditor))
          if (!InitAttemptWithoutExtension(esriLicenseProductCodeArcInfo))
          {
            // No appropriate license is available
            std::cerr << "LicenseUtilities::InitializeApp -- " 
              << "Unable to initialize ArcObjects "
              << "(no appropriate license available)." 
              << std::endl;
            return false;
          }

          return true;
  }

  // Try to init as engine, then engineGeoDB, then ArcView, 
  //    then ArcEditor, then ArcInfo 
  if (!InitAttemptWithExtension(esriLicenseProductCodeEngine,license))
    if (!InitAttemptWithExtension(esriLicenseProductCodeArcView, license))
      if (!InitAttemptWithExtension(esriLicenseProductCodeArcEditor, license))
        if (!InitAttemptWithExtension(esriLicenseProductCodeArcInfo, license))
        {
          // No appropriate license is available
          std::cerr << "LicenseUtilities::InitializeApp -- " 
            << "Unable to initialize ArcObjects "
            << "(no appropriate license available)." 
            << std::endl;
          return false;
        }

        return true;
}

// Attempt to initialize without an extension
bool InitAttemptWithoutExtension(esriLicenseProductCode product)
{
  IAoInitializePtr ipInit(CLSID_AoInitialize);

  esriLicenseStatus status = esriLicenseFailure;
  ipInit->Initialize(product, &status);
  return (status == esriLicenseCheckedOut);
}

// Attempt to initialize with an extension
bool InitAttemptWithExtension(esriLicenseProductCode product,
                              esriLicenseExtensionCode extension)
{
  IAoInitializePtr ipInit(CLSID_AoInitialize);

  esriLicenseStatus licenseStatus = esriLicenseFailure;
  ipInit->IsExtensionCodeAvailable(product, extension, &licenseStatus);
  if (licenseStatus == esriLicenseAvailable)
  {
    ipInit->Initialize(product, &licenseStatus);
    if (licenseStatus == esriLicenseCheckedOut)
      ipInit->CheckOutExtension(extension, &licenseStatus);
  }
  return (licenseStatus == esriLicenseCheckedOut);
}

// Shutdown the application and check in the license if needed.
HRESULT ShutdownApp(esriLicenseExtensionCode license)
{
  HRESULT hr;

  // Scope ipInit so released before AoUninitialize call
  {
    IAoInitializePtr ipInit(CLSID_AoInitialize);
    esriLicenseStatus status;
    if (license != NULL)
    {
      hr = ipInit->CheckInExtension(license, &status);
      if (FAILED(hr) || status != esriLicenseCheckedIn)
        std::cerr << "License Check-in failed." << std::endl;
    }
    hr = ipInit->Shutdown();
  }

  ::CoUninitialize();
  return hr;
}
