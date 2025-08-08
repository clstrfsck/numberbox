# This script
import os
from gtts import gTTS


# List of words needed to construct numbers up to four billion
number_words = [
    "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine",
    "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen",
    "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety",
    "hundred", "thousand", "million", "billion", "and"
]

# Generate files
# Create a directory to store the files
output_dir = "number_mp3_files"
os.makedirs(output_dir, exist_ok=True)

# Generate MP3 files for each word
for word in words:
    # Generate speech using gTTS
    tts = gTTS(text=word, lang='en', tld='com.au')

    # Save the speech as a MP3 file
    filename = f"{output_dir}/{word}.mp3"
    tts.save(filename)

    print(f"Generated MP3 file for '{word}'")
