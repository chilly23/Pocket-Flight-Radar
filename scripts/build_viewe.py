from pathlib import Path
import shutil, subprocess
root=Path(__file__).resolve().parents[1]
sketch=root/'build-viewe'/'PlaneRadar'
sketch.mkdir(parents=True,exist_ok=True)
shutil.copytree(root/'src',sketch/'src',dirs_exist_ok=True)
shutil.copytree(root/'include',sketch/'src',dirs_exist_ok=True)
(sketch/'PlaneRadar.ino').write_text('// Sources are compiled from src.\n')
cli=Path.home()/'AppData/Local/Programs/Arduino IDE/resources/app/lib/backend/resources/arduino-cli.exe'
fqbn='esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi'
subprocess.run([str(cli),'compile','--jobs','4','--fqbn',fqbn,'--build-property','compiler.cpp.extra_flags=-std=gnu++17 -DWM_NODEBUG -DWM_MDNS -I'+str(sketch/'src'),'--output-dir',str(root/'firmware-viewe'),str(sketch)],check=True)
