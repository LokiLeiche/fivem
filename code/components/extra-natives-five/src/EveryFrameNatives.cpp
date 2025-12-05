#include "StdInc.h"

#include <vector>
#include <algorithm>
#include <mutex>

#include "nutsnbolts.h" // for on-game-frame

#include <Local.h>
#include <ScriptEngine.h>

#include "ResourceCallbackComponent.h"

#include <scrEngine.h>
#include <Resource.h>

#include <ICoreGameInit.h>

#ifdef GTA_FIVE
#include <ScriptHandlerMgr.h>
#endif


struct disableControlActionSettings {
    fx::Resource* resource;
    int padIndex;
    int control;
};
static std::vector<disableControlActionSettings> g_disableControlActions = {};
static std::mutex g_disableControlActionsMutex;

struct disableRawKeySettings {
    fx::Resource* resource;
    int key;
};
static std::vector<disableRawKeySettings> g_disableRawKeys = {};
static std::mutex g_disableRawKeysMutex;



// class RunEveryFrameThread : public CfxThread
// {
// private:
//     bool m_shouldCreate = false;
//     std::vector<disableControlActionSettings> m_disableControlActions;
//     std::vector<disableRawKeySettings> m_disableRawKeys;
// public:
//     RunEveryFrameThread(bool shouldCreate)
//         : m_shouldCreate(shouldCreate)
//     {
//     }

//     virtual void Reset() override
//     {
//     }

//     virtual void DoRun() override
//     {
//         // Copy the lists under lock to avoid data races and to avoid
//         // holding locks while invoking native functions.
//         std::vector<disableControlActionSettings> controlCopy;
//         std::vector<disableRawKeySettings> rawCopy;

//         {
//             std::lock_guard<std::mutex> lg(g_disableControlActionsMutex);
//             controlCopy = g_disableControlActions;
//         }

//         {
//             std::lock_guard<std::mutex> lg(g_disableRawKeysMutex);
//             rawCopy = g_disableRawKeys;
//         }

//         for (const disableControlActionSettings& d : controlCopy)
//         {
//             // DISABLE_CONTROL_ACTION
//             NativeInvoke::Invoke<0xFE99B66D079CF6BC, int>(d.padIndex, d.control);
//         }

//         for (const disableRawKeySettings& d : rawCopy)
//         {
//             // DISABLE_RAW_KEY_THIS_FRAME
//             NativeInvoke::Invoke<0x8BCF0014, int>(d.key);
//         }
//     }
// };

static fx::Resource* registerStopHandler(void (fn)(fx::Resource*))
{
    fx::OMPtr<IScriptRuntime> runtime;
    if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
    {
        fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());
        resource->OnStop.Connect([resource, fn]()
        {
            fn(resource);
        });
        return resource;
    }
    return nullptr;
}

//static RunEveryFrameThread thread(false);

#include <Hooking.h>

static InitFunction initFunction([] ()
{
    fx::ScriptEngine::RegisterNativeHandler("DISABLE_CONTROL_ACTION_TOGGLE", [](fx::ScriptContext& context)
    {
        fx::Resource* resource = registerStopHandler([](fx::Resource* resource)
        {
            std::lock_guard<std::mutex> lg(g_disableControlActionsMutex);
            for (auto i = g_disableControlActions.begin(); i != g_disableControlActions.end();)
            {
                if (i->resource == resource)
                {
                    using std::swap;
                    swap(*i, g_disableControlActions.back());
                    g_disableControlActions.pop_back();
                }
                else
                {
                    ++i;
                }
            }
        });

        if (!resource)
        {
            return;
        }

        int padIndex = context.GetArgument<int>(0);
        int control = context.GetArgument<int>(1);
        bool enable = context.GetArgument<bool>(2);
        {
            std::lock_guard<std::mutex> lg(g_disableControlActionsMutex);

            for (disableControlActionSettings& d : g_disableControlActions)
            {
                if (d.padIndex == padIndex && d.control == control)
                {
                    if (enable)
                    {
                        d.resource = resource;
                    }
                    else
                    {
                        using std::swap;
                        swap(d, g_disableControlActions.back());
                        g_disableControlActions.pop_back();
                    }

                    return;
                }
            }

            if (enable)
            {
                g_disableControlActions.emplace_back(disableControlActionSettings{resource, padIndex, control});
            }
        }
    });

    fx::ScriptEngine::RegisterNativeHandler("DISABLE_RAW_KEY_TOGGLE", [](fx::ScriptContext& context)
    {
        fx::Resource* resource = registerStopHandler([](fx::Resource* resource)
        {
            std::lock_guard<std::mutex> lg(g_disableRawKeysMutex);
            for (auto i = g_disableRawKeys.begin(); i != g_disableRawKeys.end();)
            {
                if (i->resource == resource)
                {
                    using std::swap;
                    swap(*i, g_disableRawKeys.back());
                    g_disableRawKeys.pop_back();
                }
                else
                {
                    ++i;
                }
            }
        });

        if (!resource)
        {
            return;
        }

        int key = context.GetArgument<int>(0);
        bool enable = context.GetArgument<bool>(1);
        {
            std::lock_guard<std::mutex> lg(g_disableRawKeysMutex);

            for (disableRawKeySettings& d : g_disableRawKeys)
            {
                if (d.key == key)
                {
                    if (enable)
                    {
                        d.resource = resource;
                    }
                    else
                    {
                        using std::swap;
                        swap(d, g_disableRawKeys.back());
                        g_disableRawKeys.pop_back();
                    }

                    return;
                }
            }

            if (enable)
            {
                g_disableRawKeys.emplace_back(disableRawKeySettings{resource, key});
            }
        }
    });

    OnGameFrame.Connect([&]
	{
        for (const disableControlActionSettings& d : g_disableControlActions)
        {
            // DISABLE_CONTROL_ACTION
            NativeInvoke::Invoke<0xFE99B66D079CF6BC, int>(d.padIndex, d.control);
        }

        for (const disableRawKeySettings& d : g_disableRawKeys)
        {
            // DISABLE_RAW_KEY_THIS_FRAME
            NativeInvoke::Invoke<0x8BCF0014, int>(d.key);
        }
	});
	// rage::scrEngine::OnScriptInit.Connect([] ()
	// {
	// 	rage::scrEngine::CreateThread(thread.GetThread());
	// });
});
