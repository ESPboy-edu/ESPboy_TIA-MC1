import os
import glob

def convert_bins_to_c_arrays(output_filename="all_roms.h"):
    # Находим все bin файлы в текущей директории
    bin_files = glob.glob("*.bin")
    bin_files.sort()
    
    if not bin_files:
        print("Внимание: .bin файлы не найдены!")
        return

    with open(output_filename, "w") as f:
        f.write("#pragma once\n")
        f.write("#include <pgmspace.h>\n\n")
        
        for bin_file in bin_files:
            # Превращаем имя файла в валидное имя C-переменной (точки меняем на подчеркивания)
            array_name = bin_file.replace(".", "_")
            
            with open(bin_file, "rb") as bf:
                data = bf.read()
                f.write(f"// Size: {len(data)} bytes\n")
                f.write(f"const uint8_t {array_name}[{len(data)}] PROGMEM = {{\n    ")
                
                for i, byte in enumerate(data):
                    f.write(f"0x{byte:02X}, ")
                    # Перенос строки каждые 16 байт для красоты
                    if (i + 1) % 16 == 0:
                        f.write("\n    ")
            f.write("\n};\n\n")
            print(f"Сконвертирован: {bin_file} -> {array_name}")
            
    print(f"\nГотово! Все файлы собраны в {output_filename}")

if __name__ == "__main__":
    convert_bins_to_c_arrays()