#include "../config.h"
#include "../wave.h"
#include <stddef.h>
#include <stdio.h>

#include "../hw/system.h"

static u8 sound_wavetable[3][PF_SOUND_SAMPLES];

int main()
{
  int i;
  FILE *file = fopen("src/wavetables/sin.h", "w");
  if (!file)
  {
    perror("Failed to open sin.h");
    return 1;
  }

  printf("Generating %uhz wavetables with %u samples...", PF_SOUND_FREQUENCY, PF_SOUND_SAMPLES);

  fprintf(file, "/**\n"
                " * This is an autogenerated header file.\n"
                " * It contains a wavetable for a sin wave at 1000, 500, and 120hz.\n"
                " */\n\n"
                "#ifndef PRESS_F_SIN_H\n#define PRESS_F_SIN_H\n\n");
  fprintf(file, "static const unsigned char sound_wavetable[3][PF_SOUND_SAMPLES] = {\n");

  pf_generate_wavetables(sound_wavetable);

  for (int j = 0; j < 3; j++)
  {
    fprintf(file, "  { ");
    for (i = 0; i < PF_SOUND_SAMPLES; i++)
    {
      sound_wavetable[j][i] = pf_wave((2 * PF_PI * (j == 0 ? 1000 : j == 1 ? 500 : 120) * (float)i * PF_SOUND_PERIOD), FALSE) > 0 ? 1 : 0;
      fprintf(file, "%d%s", sound_wavetable[j][i], (i < PF_SOUND_SAMPLES - 1) ? ", " : "");
    }
    fprintf(file, " },\n");
  }

  fprintf(file, "};\n\n#endif\n");
  fclose(file);

  printf("done.\n");

  file = fopen("src/asm/offsets.h", "w");

  if (!file)
  {
    perror("Failed to open offsets.h");
    return 1;
  }

  printf("Generating preprocessor-friendly offsets...");

  fprintf(file, "/**\n"
                " * This is an autogenerated header file.\n"
                " * It contains structure offsets to be used with the preprocessor.\n"
                " */\n\n"
                "#ifndef PRESS_F_OFFSETS_H\n#define PRESS_F_OFFSETS_H\n\n");

  fprintf(file, "#define F8_OFFSET_PC0U %lu\n", offsetof(f8_system_t, pc0));
  fprintf(file, "#define F8_OFFSET_PC0L %lu\n", offsetof(f8_system_t, pc0));
  fprintf(file, "#define F8_OFFSET_PC1U %lu\n", offsetof(f8_system_t, pc1));
  fprintf(file, "#define F8_OFFSET_DC0U %lu\n", offsetof(f8_system_t, dc0));
  fprintf(file, "#define F8_OFFSET_DC1U %lu\n", offsetof(f8_system_t, dc1));
  fprintf(file, "#define F8_OFFSET_A %lu\n", offsetof(f8_system_t, main_cpu.accumulator));
  fprintf(file, "#define F8_OFFSET_W %lu\n", offsetof(f8_system_t, main_cpu.status_register));
  fprintf(file, "#define F8_OFFSET_ISAR %lu\n", offsetof(f8_system_t, main_cpu.isar));
  fprintf(file, "#define F8_OFFSET_SCRATCHPAD %lu\n", offsetof(f8_system_t, main_cpu.scratchpad[0]));
  fprintf(file, "#define F8_OFFSET_KU %lu\n", offsetof(f8_system_t, main_cpu.scratchpad[12]));
  fprintf(file, "#define F8_OFFSET_KL %lu\n", offsetof(f8_system_t, main_cpu.scratchpad[13]));
  fprintf(file, "#define F8_OFFSET_QU %lu\n", offsetof(f8_system_t, main_cpu.scratchpad[14]));
  fprintf(file, "#define F8_OFFSET_QL %lu\n", offsetof(f8_system_t, main_cpu.scratchpad[15]));
#if !PF_ROMC
  fprintf(file, "#define F8_OFFSET_VMEM %lu\n", offsetof(f8_system_t, memory));
#else
  fprintf(file, "#define F8_OFFSET_VMEM 0\n");
#endif

  fprintf(file, "\n#endif\n");
  fclose(file);

  printf("done.\n");

  return 0;
}
