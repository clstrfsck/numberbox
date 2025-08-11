import os
import struct
import subprocess
import numpy as np

from pathlib import Path

no_silence_trim = {
    "billion.mp3" # Sounds too much like "million" if the attack is modified
}
adpcm_rate = "22058"
format_ima_adpcm = 17

def sox(command):
    try:
        result = subprocess.run(["sox"] + command, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        print(f"Error executing Sox command: {e}")
        print(e.stderr)
        return None

def extract_data_chunk(wav_file_path):
    with open(wav_file_path, 'rb') as f:
        # Read RIFF header
        riff_header = f.read(12)
        if len(riff_header) != 12:
            raise ValueError("Invalid WAV file: too short")

        chunk_id, chunk_size, format_type = struct.unpack('<4sI4s', riff_header)

        if chunk_id != b'RIFF':
            raise ValueError(f"Not a RIFF file: {chunk_id}")
        if format_type != b'WAVE':
            raise ValueError(f"Not a WAVE file: {format_type}")

        audio_format = None
        sample_rate = -1
        channels = 1
        samples_per_block = 0
        # Parse chunks until we find the data chunk
        while True:
            chunk_header = f.read(8)
            if len(chunk_header) != 8:
                raise ValueError("Unexpected end of file while reading chunk header")

            chunk_type, chunk_data_size = struct.unpack('<4sI', chunk_header)

            if chunk_type == b'fmt ':
                # Parse format chunk to get audio info
                fmt_data = f.read(chunk_data_size)
                if len(fmt_data) < 20:
                    raise ValueError(f"Invalid fmt chunk for IMA ADPCM of length {len(fmt_data)}")

                audio_format, channels, sample_rate, avg_bytes_per_sec, block_align, bits_per_sample, cb_size, samples_per_block = struct.unpack('<HHIIHHHH', fmt_data[:20])
                #print(f"Audio Format:      {audio_format}")
                #print(f"Channels:          {channels}")
                #print(f"Sample Rate:       {sample_rate}")
                #print(f"Avg Bytes per Sec: {avg_bytes_per_sec}")
                #print(f"Block Align:       {block_align}")
                #print(f"Bits per Sample:   {bits_per_sample}")
                #print(f"Extra size:        {cb_size}")
                #print(f"Samples per block: {samples_per_block}")

            elif chunk_type == b'fact':
                # Quietly ignore this
                f.read(chunk_data_size)
                if chunk_data_size % 2 == 1:
                    f.read(1)

            elif chunk_type == b'data':
                # Found the data chunk - extract it
                if audio_format is None:
                    raise ValueError("Audio format not found")
                if audio_format != format_ima_adpcm or sample_rate != int(adpcm_rate) or channels != 1:
                    raise ValueError(f"Unsupported WAV format; format: {audio_format}, rate: {sample_rate}, channels: {channels}")

                wav_data = f.read(chunk_data_size)
                if len(wav_data) != chunk_data_size:
                    raise ValueError(f"Expected {chunk_data_size} bytes, got {len(wav_data)}")

                return wav_data, samples_per_block

            else:
                # Skip unknown chunks
                print(f"Skipping chunk: {chunk_type}")
                f.read(chunk_data_size)

                # Ensure we're on an even byte boundary (WAV chunks must be word-aligned)
                if chunk_data_size % 2 == 1:
                    f.read(1)

input_dir = Path("number_mp3_files")
output_dir = Path("number_adpcm_files")
header_dir = Path("../audio")
os.makedirs(output_dir, exist_ok=True)
os.makedirs(header_dir, exist_ok=True)

mp3_files = [file for file in input_dir.iterdir() if file.name.lower().endswith('.mp3')]

for mp3_file in mp3_files:
    raw_file = output_dir / mp3_file.with_suffix(".wav").name
    if mp3_file.name in no_silence_trim:
        print(f"{mp3_file} -> {raw_file} no processing")
        sox(
            [
                mp3_file,
                "--encoding", "ima-adpcm",
                "--rate", adpcm_rate,
                raw_file
            ]
        )
    else:
        print(f"{mp3_file} -> {raw_file} silence removed")
        sox(
            [
                mp3_file,
                "--encoding", "ima-adpcm",
                "--rate", adpcm_rate,
                raw_file,
                "silence", "1", "0.1", "0.2%",
                "reverse",
                "silence", "1", "0.1", "0.2%",
                "reverse"
            ]
        )
    raw_data, samples_per_block = extract_data_chunk(raw_file)
    adpcm_data = np.frombuffer(raw_data, dtype=np.uint8)
    header_file = header_dir / raw_file.with_suffix(".h").name
    sample_name = header_file.stem.upper().replace(".", "_")
    print(f"{raw_file} -> {header_file} as {sample_name}")
    with open(header_file, "w") as header:
        header.write(f"constexpr uint32_t {sample_name}_SAMPLE_RATE = {adpcm_rate};\n")
        header.write(f"constexpr uint32_t {sample_name}_SAMPLE_SIZE = {len(adpcm_data)};\n")
        header.write(f"constexpr uint32_t {sample_name}_SAMPLES_PER_BLOCK = {samples_per_block};\n")
        header.write("\n")
        header.write(f"const uint8_t INFLASH {sample_name}_AUDIO_DATA[] = {{")

        index = 0
        while index < len(adpcm_data):
            if index % 8 == 0:
                header.write("\n   ")
            header.write(f" 0x{adpcm_data[index]:02x}")
            if index != len(adpcm_data) - 1:
                header.write(",")
            index += 1
        header.write("\n};\n")

print("Done")
