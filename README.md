# BigInt Component/Plugin for open.mp and SA:MP

A lightweight component/plugin that adds signed 64-bit integer support to Pawn scripts for both **open.mp** and **SA:MP** servers. It exposes a `BigInt` tag plus a collection of arithmetic, conversion, and comparison natives so game-mode authors can safely store counters such as bank balances, experience totals, and economy statistics without overflowing the default 32-bit cell size.

> **Note:** SA:MP plugin support was added by [NimaBastani](https://github.com/NimaBastani).

> **Original code:** [AmyrAhmady's repository](https://github.com/AmyrAhmady/omp-bigint).
## Building

### Prerequisites

- CMake 3.19 or higher
- C++17 compatible compiler
- For open.mp: open.mp SDK (included in `deps/omp-sdk`)
- For SA:MP: Pawn AMX library (included in `deps/pawn`) and SA:MP plugin SDK (included in `deps/samp-plugin-sdk`)

### Building for open.mp (default)

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

The component will be built as `omp-bigint.dll` (Windows) or `omp-bigint.so` (Linux).

### Building for SA:MP

**Windows (32-bit):**
```bash
cmake -B build -S . -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_SAMP_PLUGIN=ON -A Win32
cmake --build build
```

**Windows (64-bit):**
```bash
cmake -B build -S . -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_SAMP_PLUGIN=ON
cmake --build build
```

**Linux:**
```bash
cmake -B build -S . -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_SAMP_PLUGIN=ON
cmake --build build
```

The plugin will be built as `samp-bigint.dll` (Windows) or `samp-bigint.so` (Linux).

## Installation

### For open.mp

1. Visit the **Releases** page of this repository and download the archive or binary that matches your server platform (Windows `.dll`, Linux `.so`).
2. Extract the download and copy the component file into your open.mp server `components/` directory.
3. Copy `omp_bigint.inc` into the include directory used by your Pawn compiler (e.g., `gamemodes/include/`).
4. Add `#include <omp_bigint>` to any script that needs the BigInt natives and rebuild your Pawn scripts.

### For SA:MP

1. Visit the **Releases** page of this repository and download the SA:MP plugin binary that matches your server platform (Windows `.dll`, Linux `.so`).
2. Copy the plugin file into your SA:MP server `plugins/` directory.
3. Add the plugin to your `server.cfg`:
   ```
   plugins samp-bigint
   ```
   (On Windows, use `plugins samp-bigint.dll`; on Linux, use `plugins samp-bigint.so`)
4. Copy `omp_bigint.inc` into the include directory used by your Pawn compiler (e.g., `pawno/include/`).
5. Add `#include <omp_bigint>` to any script that needs the BigInt natives and rebuild your Pawn scripts.

## Usage

The include defines the `BigInt` tag plus helpers such as `new_bigint`, `BigInt_SetInt`, and the arithmetic/compare natives. A quick example that tracks per-player bank balances:

```pawn
#include <omp_bigint>

new_bigint(g_BankBalance[MAX_PLAYERS]);

/*
 Or if you want to use the common way of creating variables:
 
     new BigInt:g_BankBalance[MAX_PLAYERS][eBigIntParts];

     new BigInt:TestVar[eBigIntParts];
 or
     new_bigint(TestVar);

 or in your enum:
     enum PLAYER_DATA
     {
        ...
        BigInt:BankMoney[eBigIntParts],
        ...
     };
*/
 

public OnPlayerConnect(playerid)
{
    BigInt_SetInt(g_BankBalance[playerid], 0);
}

stock BankDeposit(playerid, amount)
{
    BigInt_AddInt(g_BankBalance[playerid], amount);
}

CMD:balance(playerid)
{
    new buf[32];
    BigInt_ToString(g_BankBalance[playerid], buf, sizeof buf);
    SendClientMessage(playerid, -1, buf);
    return 1;
}
```

All operations return simple success/failure flags (e.g., division by zero yields `false`) so you can guard your logic as needed.

### BigIntHandle: Operator Overloading Support

> **Note:** Handle system and operator overloading support was added by [NimaBastani](https://github.com/NimaBastani).

Since Pawn doesn't support operator overloading for array types, a `BigIntHandle` wrapper system has been implemented. `BigIntHandle` is a simple cell type that can be used with standard operators (`+`, `-`, `*`, `/`, `%`, `==`, `!=`, `>`, `<`, `>=`, `<=`, `++`, `--`, `!`), making BigInt operations more intuitive and readable.

#### Basic Usage with Operators

```pawn
#include <omp_bigint>

public OnGameModeInit()
{
    // Create handles
    new BigIntHandle:balance = BigIntHandle_FromInt(1000);
    new BigIntHandle:deposit = BigIntHandle_FromInt(500);
    
    // Use operators naturally
    new BigIntHandle:newBalance = balance + deposit;  // 1000 + 500 = 1500
    new BigIntHandle:doubleBalance = balance * 2;     // 1000 * 2 = 2000
    new BigIntHandle:halfBalance = balance / 2;       // 1000 / 2 = 500
    
    // Comparisons
    if (balance > deposit)
        printf("Balance is greater than deposit");
    
    if (newBalance == 1500)
        printf("New balance is exactly 1500");
    
    // Increment/Decrement
    balance++;  // balance = 1001
    balance--;  // balance = 1000
    
    // Mixed operations with integers
    new BigIntHandle:result1 = balance + 100;        // 1000 + 100 = 1100
    new BigIntHandle:result2 = 2000 - balance;       // 2000 - 1000 = 1000
    new BigIntHandle:result3 = balance * 3;           // 1000 * 3 = 3000
    new BigIntHandle:result4 = 5000 / balance;       // 5000 / 1000 = 5
    
    // Complex expressions
    new BigIntHandle:complex = (balance + deposit) * 2 - 100;  // (1000 + 500) * 2 - 100 = 2900
    
    // Don't forget to free handles when done
    BigIntHandle_Free(balance);
    BigIntHandle_Free(deposit);
    BigIntHandle_Free(newBalance);
    BigIntHandle_Free(doubleBalance);
    BigIntHandle_Free(halfBalance);
    BigIntHandle_Free(result1);
    BigIntHandle_Free(result2);
    BigIntHandle_Free(result3);
    BigIntHandle_Free(result4);
    BigIntHandle_Free(complex);
    
    return 1;
}
```

#### Converting Between Handles and Arrays

```pawn
#include <omp_bigint>

public Example()
{
    // Create a BigInt array
    new_bigint(arr);
    BigInt_FromInt(arr, 12345);
    
    // Convert array to handle for operator usage
    new BigIntHandle:h = BigInt_ToHandle(arr);
    
    // Use operators
    new BigIntHandle:doubled = h * 2;  // 12345 * 2 = 24690
    
    // Convert back to array if needed
    new_bigint(result);
    BigInt_FromHandle(doubled, result);
    
    // Convert to string
    new str[32];
    BigInt_ToString(result, str, sizeof(str));
    printf("Result: %s", str);  // Output: "24690"
    
    // Clean up
    BigIntHandle_Free(h);
    BigIntHandle_Free(doubled);
}
```

#### Working with 64-bit Values

```pawn
#include <omp_bigint>

public Example64Bit()
{
    // Create large 64-bit values using FromString
    new_bigint(temp);
    BigInt_FromString(temp, "5000000000");  // 5 billion
    new BigIntHandle:fiveBillion = BigInt_ToHandle(temp);
    
    BigInt_FromString(temp, "10000000000");  // 10 billion
    new BigIntHandle:tenBillion = BigInt_ToHandle(temp);
    
    // All operators work with 64-bit values
    new BigIntHandle:sum = fiveBillion + fiveBillion;      // 10 billion
    new BigIntHandle:product = fiveBillion * 2;             // 10 billion
    new BigIntHandle:quotient = tenBillion / 2;             // 5 billion
    
    // Comparisons work correctly
    if (tenBillion > fiveBillion)
        printf("10 billion is greater than 5 billion");
    
    if (sum == tenBillion)
        printf("Sum equals ten billion");
    
    // Convert to string for display
    new str[64];
    BigInt_FromHandle(tenBillion, temp);
    BigInt_ToString(temp, str, sizeof(str));
    printf("Ten billion: %s", str);
    
    // Clean up
    BigIntHandle_Free(fiveBillion);
    BigIntHandle_Free(tenBillion);
    BigIntHandle_Free(sum);
    BigIntHandle_Free(product);
    BigIntHandle_Free(quotient);
}
```

#### Available Operators

All standard arithmetic and comparison operators are supported:

**Arithmetic:**
- `+` - Addition (BigIntHandle + BigIntHandle, BigIntHandle + int, int + BigIntHandle)
- `-` - Subtraction (BigIntHandle - BigIntHandle, BigIntHandle - int, int - BigIntHandle, -BigIntHandle)
- `*` - Multiplication (BigIntHandle * BigIntHandle, BigIntHandle * int, int * BigIntHandle)
- `/` - Division (BigIntHandle / BigIntHandle, BigIntHandle / int, int / BigIntHandle)
- `%` - Modulo (BigIntHandle % BigIntHandle, BigIntHandle % int, int % BigIntHandle)
- `++` - Increment (BigIntHandle++)
- `--` - Decrement (BigIntHandle--)

**Comparison:**
- `==` - Equality (BigIntHandle == BigIntHandle, BigIntHandle == int, int == BigIntHandle)
- `!=` - Inequality (BigIntHandle != BigIntHandle, BigIntHandle != int, int != BigIntHandle)
- `>` - Greater than (BigIntHandle > BigIntHandle, BigIntHandle > int, int > BigIntHandle)
- `<` - Less than (BigIntHandle < BigIntHandle, BigIntHandle < int, int < BigIntHandle)
- `>=` - Greater than or equal (BigIntHandle >= BigIntHandle, BigIntHandle >= int, int >= BigIntHandle)
- `<=` - Less than or equal (BigIntHandle <= BigIntHandle, BigIntHandle <= int, int <= BigIntHandle)

**Logical:**
- `!` - Logical NOT (!BigIntHandle) - returns true if value is zero

#### Important Notes

- **Memory Management:** Always free handles using `BigIntHandle_Free()` when you're done with them to prevent memory leaks.
- **Handle Pool:** The system uses a global pool of 2048 handles. If you exceed this limit, `BigIntHandle_Alloc()` will return an invalid handle (`-1`).
- **32-bit Conversion:** `BigIntHandle_ToInt()` will clamp values to 32-bit range. For full 64-bit values, use `BigInt_ToString()` or convert to array and use `BigInt_GetParts()`.
- **Division by Zero:** Division and modulo operations return an invalid handle (`-1`) when dividing by zero, preventing crashes.

## Contributing

Contributions are welcome! To get started:

- Fork the repository and create a branch for your change.
- Keep commits focused; include tests or Pawn snippets that demonstrate new or fixed behavior when possible.
- Run the CMake build locally to ensure the component/plugin still compiles on your platform.
- Test on both open.mp and SA:MP if possible.
- Open a pull request against `master`, describing the motivation, implementation details, and any testing performed.

If you encounter issues or have feature ideas, feel free to open a GitHub issue with as much detail as possible.
