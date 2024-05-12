#include <Logger.h>
#include <nativebridge/native_bridge.h>

#include <dlfcn.h>
#include <sys/system_properties.h>

#include <string>

namespace android
{
    namespace native_bridge
    {
        namespace detail
        {
            static void *backendHandle = nullptr;

            NativeBridgeCallbacks *GetCallbacksInternal();
        }

        namespace v1
        {
            bool Initialize(const NativeBridgeRuntimeCallbacks *androidRuntimeCallbacks, const char *appCodeCacheDir, const char *isa);

            void *LoadLibrary(const char *libraryPath, int flag);

            void *GetTrampoline(void *handle, const char *name, const char *shorty, uint32_t len);

            bool IsSupported(const char *libraryPath);

            const NativeBridgeRuntimeValues *GetAppEnv(const char *abi);
        }

        namespace v2
        {
            bool IsCompatibleWith(uint32_t version);

            NativeBridgeSignalHandlerFn GetSignalHandler(int signal);
        }

        namespace v3
        {
            int UnLoadLibrary(void *handle);

            const char *GetError();

            bool IsPathSupported(const char *path);

            bool InitAnonymousNamespace(const char *publicNamespaceSoNames, const char *anonNamespaceLibraryPath);

            native_bridge_namespace_t *CreateNamespace(const char *name, const char *loadLibraryPath, const char *defaultLibraryPath, uint64_t type, const char *permittedWhenIsolatedPath, native_bridge_namespace_t *parentNamespace);

            bool LinkNamespaces(native_bridge_namespace_t *from, native_bridge_namespace_t *to, const char *sharedLibsSoNames);

            void *LoadLibraryExt(const char *libraryPath, int flag, struct native_bridge_namespace_t *loadToNamespace);
        }

        namespace v4
        {
            native_bridge_namespace_t *GetVendorNamespace();

            native_bridge_namespace_t *GetExportedNamespace(const char *name);
        }
    }

    extern "C"
    {
        NativeBridgeCallbacks NativeBridgeItf = {
            // v1
            .version = 4,
            .initialize = native_bridge::v1::Initialize,
            .loadLibrary = native_bridge::v1::LoadLibrary,
            .getTrampoline = native_bridge::v1::GetTrampoline,
            .isSupported = native_bridge::v1::IsSupported,
            .getAppEnv = native_bridge::v1::GetAppEnv,
            // v2
            .isCompatibleWith = native_bridge::v2::IsCompatibleWith,
            .getSignalHandler = native_bridge::v2::GetSignalHandler,
            // v3
            .unloadLibrary = native_bridge::v3::UnLoadLibrary,
            .getError = native_bridge::v3::GetError,
            .isPathSupported = native_bridge::v3::IsPathSupported,
            .initAnonymousNamespace = native_bridge::v3::InitAnonymousNamespace,
            .createNamespace = native_bridge::v3::CreateNamespace,
            .linkNamespaces = native_bridge::v3::LinkNamespaces,
            .loadLibraryExt = native_bridge::v3::LoadLibraryExt,
            // v4
            .getVendorNamespace = native_bridge::v4::GetVendorNamespace,
            .getExportedNamespace = native_bridge::v4::GetExportedNamespace,
        };
    }
}

void __attribute__((destructor)) UnInitialize()
{
    if (android::native_bridge::detail::backendHandle)
    {
        dlclose(android::native_bridge::detail::backendHandle);
        android::native_bridge::detail::backendHandle = nullptr;
    }
}

android::NativeBridgeCallbacks *android::native_bridge::detail::GetCallbacksInternal()
{
    static android::NativeBridgeCallbacks *callbacksInterface = nullptr;

    if (nullptr != callbacksInterface)
        return callbacksInterface;

    if (nullptr != backendHandle)
        return (callbacksInterface = reinterpret_cast<android::NativeBridgeCallbacks *>(dlsym(backendHandle, "NativeBridgeItf")));

#ifdef __LP64__
    std::string backendPath = "/system/lib64/";
#else
    std::string backendPath = "/system/lib/";
#endif

    char backendName[1024]{};
    auto backendNameLength = __system_property_get("ro.dalvik.vm.native.bridge", backendName);
    if (0 < backendNameLength)
        backendPath.append(backendName);
    else
        backendPath.append("libhoudini.so");

    backendHandle = dlopen(backendPath.data(), RTLD_LAZY);
    if (nullptr == backendHandle)
        LogError("[-] Unable to load native-bridge backend %s", backendPath.data());

    callbacksInterface = reinterpret_cast<android::NativeBridgeCallbacks *>(dlsym(backendHandle, "NativeBridgeItf"));
    if (nullptr != callbacksInterface)
        LogInfo("[=] Found native-bridge handle:%p version:%u", callbacksInterface, callbacksInterface->version);

    return callbacksInterface;
}

bool android::native_bridge::v1::Initialize(const android::NativeBridgeRuntimeCallbacks *androidRuntimeCallbacks, const char *appCodeCacheDir, const char *isa)
{
    auto callbacks = detail::GetCallbacksInternal();

    if (nullptr == callbacks)
        return false;

    auto result = callbacks->initialize(androidRuntimeCallbacks, appCodeCacheDir, isa);
    if (!result)
    {
        isa = "arm64";

        LogError("[-] [Initialize] Tried default params failed, also hijack to %p %s %s", androidRuntimeCallbacks, appCodeCacheDir, isa);
        result = callbacks->initialize(androidRuntimeCallbacks, appCodeCacheDir, isa);
        if (!result)
            LogError("[-] [Initialize] Tried hijack params failed, unknow error");
        else
            LogInfo("[+] [Initialize] Tried hijack params succeeded");
    }

    return result;
}

void *android::native_bridge::v1::LoadLibrary(const char *libraryPath, int flag)
{
    auto callbacks = detail::GetCallbacksInternal();

    if (nullptr == callbacks)
        return nullptr;

    return callbacks->loadLibrary(libraryPath, flag);
}

void *android::native_bridge::v1::GetTrampoline(void *handle, const char *name, const char *shorty, uint32_t len)
{
    auto callbacks = detail::GetCallbacksInternal();

    if (nullptr == callbacks)
        return nullptr;

    return callbacks->getTrampoline(handle, name, shorty, len);
}

bool android::native_bridge::v1::IsSupported(const char *libraryPath)
{
    auto callbacks = detail::GetCallbacksInternal();

    if (nullptr == callbacks)
        return false;

    return callbacks->isSupported(libraryPath);
}

const android::NativeBridgeRuntimeValues *android::native_bridge::v1::GetAppEnv(const char *abi)
{
    auto callbacks = detail::GetCallbacksInternal();

    if (nullptr == callbacks)
        return nullptr;

    return callbacks->getAppEnv(abi);
}

bool android::native_bridge::v2::IsCompatibleWith(uint32_t version)
{
    auto callbacks = detail::GetCallbacksInternal();

    if (nullptr == callbacks)
        return false;

    return callbacks->isCompatibleWith(version);
}

android::NativeBridgeSignalHandlerFn android::native_bridge::v2::GetSignalHandler(int signal)
{
    auto callbacks = detail::GetCallbacksInternal();

    if (nullptr == callbacks)
        return nullptr;

    return callbacks->getSignalHandler(signal);
}

int android::native_bridge::v3::UnLoadLibrary(void *handle)
{
    auto callbacks = detail::GetCallbacksInternal();

    if (nullptr == callbacks)
        return -1;

    return callbacks->unloadLibrary(handle);
}

const char *android::native_bridge::v3::GetError()
{
    auto callbacks = detail::GetCallbacksInternal();

    if (nullptr == callbacks)
        return nullptr;

    return callbacks->getError();
}

bool android::native_bridge::v3::IsPathSupported(const char *path)
{
    auto callbacks = detail::GetCallbacksInternal();

    if (nullptr == callbacks)
        return false;

    return callbacks->isPathSupported(path);
}

bool android::native_bridge::v3::InitAnonymousNamespace(const char *publicNamespaceSoNames, const char *anonNamespaceLibraryPath)
{
    auto callbacks = detail::GetCallbacksInternal();

    if (nullptr == callbacks)
        return false;

    return callbacks->initAnonymousNamespace(publicNamespaceSoNames, anonNamespaceLibraryPath);
}

android::native_bridge_namespace_t *android::native_bridge::v3::CreateNamespace(const char *name, const char *loadLibraryPath, const char *defaultLibraryPath, uint64_t type, const char *permittedWhenIsolatedPath, android::native_bridge_namespace_t *parentNamespace)
{
    auto callbacks = detail::GetCallbacksInternal();

    if (nullptr == callbacks)
        return nullptr;

    return callbacks->createNamespace(name, loadLibraryPath, defaultLibraryPath, type, permittedWhenIsolatedPath, parentNamespace);
}

bool android::native_bridge::v3::LinkNamespaces(android::native_bridge_namespace_t *from, android::native_bridge_namespace_t *to, const char *sharedLibsSoNames)
{
    auto callbacks = detail::GetCallbacksInternal();

    if (nullptr == callbacks)
        return false;

    return callbacks->linkNamespaces(from, to, sharedLibsSoNames);
}

void *android::native_bridge::v3::LoadLibraryExt(const char *libraryPath, int flag, struct android::native_bridge_namespace_t *loadToNamespace)
{
    auto callbacks = detail::GetCallbacksInternal();

    if (nullptr == callbacks)
        return nullptr;

    return callbacks->loadLibraryExt(libraryPath, flag, loadToNamespace);
}

android::native_bridge_namespace_t *android::native_bridge::v4::GetVendorNamespace()
{
    auto callbacks = detail::GetCallbacksInternal();

    if (nullptr == callbacks)
        return nullptr;

    return callbacks->getVendorNamespace();
}

android::native_bridge_namespace_t *android::native_bridge::v4::GetExportedNamespace(const char *name)
{
    auto callbacks = detail::GetCallbacksInternal();

    if (nullptr == callbacks)
        return nullptr;

    return callbacks->getExportedNamespace(name);
}
