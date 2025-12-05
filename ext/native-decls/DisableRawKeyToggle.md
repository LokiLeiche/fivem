---
ns: CFX
apiset: client
---
## DISABLE_RAW_KEY_TOGGLE

```c
void DISABLE_RAW_KEY_TOGGLE(int rawKeyIndex, BOOL disable);
```

Disables the specified `rawKeyIndex`, making it not trigger the regular `IS_RAW_KEY_*` natives.

Only has to be called once, not every frame.

Virtual key codes can be found [here](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)

## Parameters
* **rawKeyIndex**: Index of raw key from keyboard.
* **disable**: Enable/Disable the raw key

## Examples
```lua
local KEY_SPACE = 32
DisableRawKeyToggle(KEY_SPACE, true)
-- This will not get triggered
if IsRawKeyDown(KEY_SPACE) then
	print("unreachable :(")
end
-- this will get triggered
if IsDisabledRawKeyDown(KEY_SPACE) then
    print("Spacebar is down")
end

-- later in code enable the key again
DisableRawKeyToggle(KEY_SPACE, false)
```
