SET PATH=C:\Programs\Keil_v5\ARM\ARMCC\Bin;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Program Files (x86)\Kaspersky Lab\Kaspersky Endpoint Security 10 for Windows SP1\;C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Programs\Git\cmd
SET CPU_TYPE=STM32F070RB
SET CPU_VENDOR=STMicroelectronics
SET UV2_TARGET=DS1820_HelloWorld
SET CPU_CLOCK=0x01F78A40
"C:\Programs\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\ds1820.__i"
"C:\Programs\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\linkedlist.__i"
"C:\Programs\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\main.__i"
"C:\Programs\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\one_wire.__i"
"C:\Programs\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\one_wire_device.__i"
"C:\Programs\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\y_ds1820.__i"
"C:\Programs\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\ylist.__i"
"C:\Programs\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\crc.__i"
"C:\Programs\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\bin2hex.__i"
"C:\Programs\Keil_v5\ARM\ARMCC\Bin\ArmLink" --Via ".\BUILD\DS1820_HelloWorld.lnp"
"C:\Programs\Keil_v5\ARM\ARMCC\Bin\fromelf.exe" ".\BUILD\DS1820_HelloWorld.axf" --i32combined --output ".\BUILD\DS1820_HelloWorld.hex"
