import os
import subprocess
import numpy as np

from pathlib import Path

no_silence_trim = {
    "billion.mp3" # Sounds too much like "million" if the attack is modified
}
raw_rate = "22050"

def sox(command):
    try:
        result = subprocess.run(["sox"] + command, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        print(f"Error executing Sox command: {e}")
        print(e.stderr)
        return None

input_dir = Path("number_mp3_files")
output_dir = Path("number_raw_files")
header_dir = Path("..")
os.makedirs(output_dir, exist_ok=True)
os.makedirs(header_dir, exist_ok=True)

mp3_files = [file for file in input_dir.iterdir() if file.name.lower().endswith('.mp3')]

for mp3_file in mp3_files:
    raw_file = output_dir / mp3_file.with_suffix(".raw").name
    if mp3_file.name in no_silence_trim:
        print(f"{mp3_file} -> {raw_file} no processing")
        sox(
            [
                mp3_file,
                "--rate", raw_rate,
                "--bits", "16",
                "--encoding", "signed-integer",
                "--endian", "little",
                raw_file
            ]
        )
    else:
        print(f"{mp3_file} -> {raw_file} silence removed")
        sox(
            [
                mp3_file,
                "--rate", raw_rate,
                "--bits", "16",
                "--encoding", "signed-integer",
                "--endian", "little",
                raw_file,
                "silence", "1", "0.1", "0.2%",
                "reverse",
                "silence", "1", "0.1", "0.2%",
                "reverse"
            ]
        )
    raw_data = np.fromfile(raw_file, dtype=np.uint8)
    header_file = header_dir / raw_file.with_suffix(".h").name
    sample_name = header_file.stem.upper()
    print(f"{raw_file} -> {header_file} as {sample_name}")
    with open(header_file, "w") as header:
        header.write(f"const uint32_t {sample_name}_SAMPLE_RATE = {raw_rate};\n")
        header.write(f"const uint32_t {sample_name}_SAMPLE_SIZE = {len(raw_data)};\n")
        header.write(f"const uint32_t {sample_name}_SAMPLES_PER_BLOCK = {len(raw_data) // 2};\n")
        header.write("\n")
        header.write(f"const uint8_t {sample_name}_AUDIO_DATA[] = {{")

        index = 0
        while index < len(raw_data):
            if index % 8 == 0:
                header.write("\n   ")
            header.write(f" 0x{raw_data[index]:02x}")
            if index != len(raw_data) - 1:
                header.write(",")
            index += 1
        header.write("\n};\n")

print("Done")
