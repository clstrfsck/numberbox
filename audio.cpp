#include "audio.h"

// Order here is important for __in_flash/INFLASH definition
#include "pico.h"
#define INFLASH __in_flash("samples")

#include "audio/and.h"
#include "audio/billion.h"
#include "audio/eight.h"
#include "audio/eighteen.h"
#include "audio/eighty.h"
#include "audio/eleven.h"
#include "audio/fifteen.h"
#include "audio/fifty.h"
#include "audio/five.h"
#include "audio/forty.h"
#include "audio/four.h"
#include "audio/fourteen.h"
#include "audio/hundred.h"
#include "audio/million.h"
#include "audio/nine.h"
#include "audio/nineteen.h"
#include "audio/ninety.h"
#include "audio/one.h"
#include "audio/seven.h"
#include "audio/seventeen.h"
#include "audio/seventy.h"
#include "audio/six.h"
#include "audio/sixteen.h"
#include "audio/sixty.h"
#include "audio/ten.h"
#include "audio/thirteen.h"
#include "audio/thirty.h"
#include "audio/thousand.h"
#include "audio/three.h"
#include "audio/twelve.h"
#include "audio/twenty.h"
#include "audio/two.h"
#include "audio/zero.h"

namespace {
    const audio::sample_data number_samples[] = {
        [join_and] = { AND_AUDIO_DATA, AND_SAMPLE_SIZE, AND_SAMPLES_PER_BLOCK },
        [billion] = { BILLION_AUDIO_DATA, BILLION_SAMPLE_SIZE, BILLION_SAMPLES_PER_BLOCK },
        [eight] = { EIGHT_AUDIO_DATA, EIGHT_SAMPLE_SIZE, EIGHT_SAMPLES_PER_BLOCK },
        [eighteen] = { EIGHTEEN_AUDIO_DATA, EIGHTEEN_SAMPLE_SIZE, EIGHTEEN_SAMPLES_PER_BLOCK },
        [eighty] = { EIGHTY_AUDIO_DATA, EIGHTY_SAMPLE_SIZE, EIGHTY_SAMPLES_PER_BLOCK },
        [eleven] = { ELEVEN_AUDIO_DATA, ELEVEN_SAMPLE_SIZE, ELEVEN_SAMPLES_PER_BLOCK },
        [fifteen] = { FIFTEEN_AUDIO_DATA, FIFTEEN_SAMPLE_SIZE, FIFTEEN_SAMPLES_PER_BLOCK },
        [fifty] = { FIFTY_AUDIO_DATA, FIFTY_SAMPLE_SIZE, FIFTY_SAMPLES_PER_BLOCK },
        [five] = { FIVE_AUDIO_DATA, FIVE_SAMPLE_SIZE, FIVE_SAMPLES_PER_BLOCK },
        [forty] = { FORTY_AUDIO_DATA, FORTY_SAMPLE_SIZE, FORTY_SAMPLES_PER_BLOCK },
        [four] = { FOUR_AUDIO_DATA, FOUR_SAMPLE_SIZE, FOUR_SAMPLES_PER_BLOCK },
        [fourteen] = { FOURTEEN_AUDIO_DATA, FOURTEEN_SAMPLE_SIZE, FOURTEEN_SAMPLES_PER_BLOCK },
        [hundred] = { HUNDRED_AUDIO_DATA, HUNDRED_SAMPLE_SIZE, HUNDRED_SAMPLES_PER_BLOCK },
        [million] = { MILLION_AUDIO_DATA, MILLION_SAMPLE_SIZE, MILLION_SAMPLES_PER_BLOCK },
        [nine] = { NINE_AUDIO_DATA, NINE_SAMPLE_SIZE, NINE_SAMPLES_PER_BLOCK },
        [nineteen] = { NINETEEN_AUDIO_DATA, NINETEEN_SAMPLE_SIZE, NINETEEN_SAMPLES_PER_BLOCK },
        [ninety] = { NINETY_AUDIO_DATA, NINETY_SAMPLE_SIZE, NINETY_SAMPLES_PER_BLOCK },
        [one] = { ONE_AUDIO_DATA, ONE_SAMPLE_SIZE, ONE_SAMPLES_PER_BLOCK },
        [seven] = { SEVEN_AUDIO_DATA, SEVEN_SAMPLE_SIZE, SEVEN_SAMPLES_PER_BLOCK },
        [seventeen] = { SEVENTEEN_AUDIO_DATA, SEVENTEEN_SAMPLE_SIZE, SEVENTEEN_SAMPLES_PER_BLOCK },
        [seventy] = { SEVENTY_AUDIO_DATA, SEVENTY_SAMPLE_SIZE, SEVENTY_SAMPLES_PER_BLOCK },
        [six] = { SIX_AUDIO_DATA, SIX_SAMPLE_SIZE, SIX_SAMPLES_PER_BLOCK },
        [sixteen] = { SIXTEEN_AUDIO_DATA, SIXTEEN_SAMPLE_SIZE, SIXTEEN_SAMPLES_PER_BLOCK },
        [sixty] = { SIXTY_AUDIO_DATA, SIXTY_SAMPLE_SIZE, SIXTY_SAMPLES_PER_BLOCK },
        [ten] = { TEN_AUDIO_DATA, TEN_SAMPLE_SIZE, TEN_SAMPLES_PER_BLOCK },
        [thirteen] = { THIRTEEN_AUDIO_DATA, THIRTEEN_SAMPLE_SIZE, THIRTEEN_SAMPLES_PER_BLOCK },
        [thirty] = { THIRTY_AUDIO_DATA, THIRTY_SAMPLE_SIZE, THIRTY_SAMPLES_PER_BLOCK },
        [thousand] = { THOUSAND_AUDIO_DATA, THOUSAND_SAMPLE_SIZE, THOUSAND_SAMPLES_PER_BLOCK },
        [three] = { THREE_AUDIO_DATA, THREE_SAMPLE_SIZE, THREE_SAMPLES_PER_BLOCK },
        [twelve] = { TWELVE_AUDIO_DATA, TWELVE_SAMPLE_SIZE, TWELVE_SAMPLES_PER_BLOCK },
        [twenty] = { TWENTY_AUDIO_DATA, TWENTY_SAMPLE_SIZE, TWENTY_SAMPLES_PER_BLOCK },
        [two] = { TWO_AUDIO_DATA, TWO_SAMPLE_SIZE, TWO_SAMPLES_PER_BLOCK },
        [zero] = { ZERO_AUDIO_DATA, ZERO_SAMPLE_SIZE, ZERO_SAMPLES_PER_BLOCK }
    };
    constexpr size_t number_samples_size = sizeof(number_samples) / sizeof(number_samples[0]);
}

namespace audio {
    const sample_data &get_sample_data(number_token index) {
        if (index < 0 || index >= number_samples_size) {
            static const sample_data empty_sample{ nullptr, 0, 0 };
            return empty_sample;
        }
        return number_samples[index];
    }
}
