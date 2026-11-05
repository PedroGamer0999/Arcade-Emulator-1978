#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cpu8080.h"
#include "rom_manager.h"
#include "game_driver.h"

static int test_total = 0;
static int test_passed = 0;

#define ASSERT_TEST(cond, msg) do { \
    test_total++; \
    if (cond) { \
        test_passed++; \
        printf("  [PASS] %s\n", msg); \
    } else { \
        printf("  [FAIL] %s (linha %d)\n", msg, __LINE__); \
    } \
} while(0)

/* Teste 1: Cálculo de CRC32 conhecido (IEEE 802.3) */
static void test_crc32_calculation(void) {
    printf("\n--- Teste 1: Calculo de CRC32 (IEEE 802.3) ---\n");
    const uint8_t test_data[] = "123456789";
    uint32_t crc = calcular_crc32(test_data, 9);
    /* Valor padrão de verificação para "123456789" é 0xCBF43926 */
    ASSERT_TEST(crc == 0xCBF43926u, "CRC32 da string '123456789' confere com valor padrao");
}

/* Teste 2: Registro e consulta de Game Drivers */
static void test_game_drivers_registry(void) {
    printf("\n--- Teste 2: Registro e Consulta de Game Drivers ---\n");
    const GameDriver **lista = NULL;
    int count = listar_drivers_disponiveis(&lista);
    ASSERT_TEST(count == 6, "Exatamente 6 drivers registrados (invaders, invadpt2, lrescu, ballbomb, schaser, cpudiag)");

    const GameDriver *inv = obter_driver_por_id("invaders");
    ASSERT_TEST(inv != NULL, "obter_driver_por_id('invaders') encontrado");
    if (inv) {
        ASSERT_TEST(inv->romset != NULL && inv->romset->num_pecas == 4, "Driver invaders possui 4 pecas de ROM");
    }

    const GameDriver *lrescu = obter_driver_por_id("lrescu");
    ASSERT_TEST(lrescu != NULL, "obter_driver_por_id('lrescu') encontrado");

    const GameDriver *cpudiag = obter_driver_por_id("cpudiag");
    ASSERT_TEST(cpudiag != NULL && cpudiag->cpudiag_mode == 1, "Driver cpudiag possui flag cpudiag_mode=1");

    const GameDriver *by_file = detectar_driver_por_arquivo("roms/lrescu.zip");
    ASSERT_TEST(by_file == lrescu, "detectar_driver_por_arquivo('roms/lrescu.zip') detecta lrescu");
}

/* Teste 3: Carregamento de ROM a partir de pasta solta */
static void test_romset_load_folder(void) {
    printf("\n--- Teste 3: Carga de ROM da pasta solta (rom/) ---\n");
    State8080 cpu;
    memset(&cpu, 0, sizeof(cpu));

    const GameDriver *inv = obter_driver_por_id("invaders");
    assert(inv != NULL);

    int res = carregar_romset(&cpu, inv->romset, "rom");
    ASSERT_TEST(res == 0, "carregar_romset() com pasta 'rom' retorna sucesso");

    /* Verifica se os bytes da primeira instrução estão carregados */
    /* Em Space Invaders, 0x0000-0x0002 são NOPs (0x00), e 0x0003 é JMP 0x18D4 (0xC3, 0xD4, 0x18) */
    ASSERT_TEST(cpu.memory[3] == 0xC3 && cpu.memory[4] == 0xD4 && cpu.memory[5] == 0x18,
                "Memoria de ROM em 0x0003 contem salto inicial (JMP 0x18D4)");
}

/* Teste 4: Carregamento de ROM a partir de arquivo ZIP via miniz */
static void test_romset_load_zip(void) {
    printf("\n--- Teste 4: Carga de ROM a partir de arquivo ZIP (build/invaders.zip) ---\n");
    State8080 cpu;
    memset(&cpu, 0, sizeof(cpu));

    const GameDriver *inv = obter_driver_por_id("invaders");
    assert(inv != NULL);

    int res = carregar_romset(&cpu, inv->romset, "build/invaders.zip");
    ASSERT_TEST(res == 0, "carregar_romset() com arquivo ZIP retorna sucesso");
}

/* Teste 5: Rejeição amigável com arquivo ausente */
static void test_romset_missing_file(void) {
    printf("\n--- Teste 5: Tratamento de arquivo ou pasta ausente ---\n");
    State8080 cpu;
    memset(&cpu, 0, sizeof(cpu));

    const GameDriver *inv = obter_driver_por_id("invaders");
    assert(inv != NULL);

    int res = carregar_romset(&cpu, inv->romset, "caminho_inexistente_1234");
    ASSERT_TEST(res != 0, "carregar_romset() falha graciosamente se arquivo/pasta nao existe");
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    printf("============================================================\n");
    printf("      SUITE DE TESTES - SPRINT 4: ROM MANAGER & DRIVERS     \n");
    printf("============================================================\n");

    test_crc32_calculation();
    test_game_drivers_registry();
    test_romset_load_folder();
    test_romset_load_zip();
    test_romset_missing_file();

    printf("\n============================================================\n");
    printf("  RESULTADO: %d/%d testes passaram (%.1f%%)\n",
           test_passed, test_total, (float)test_passed / (float)test_total * 100.0f);
    printf("============================================================\n");

    return (test_passed == test_total) ? 0 : 1;
}
