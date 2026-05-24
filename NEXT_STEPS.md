# NEXT_STEPS - Tarefas Pendentes para Continuação

**Dispositivo:** RedMagic 11 Pro (NX809J) — Snapdragon 8 Gen 5 (SM8850)  
**Última atualização:** 2026-05-24

---

## Estado Atual — O Que Já Está Pronto

### Techpacks Compilados (9/9 disponíveis) — 49 módulos .ko ✅
```
audio-kernel       → 35 .ko (swr_haptics, lpass_cdc, wcd939x, aw882xx, etc.)
camera-kernel      →  1 .ko (camera.ko)
display-drivers    →  1 .ko (msm_drm.ko — 39MB)
graphics-kernel    →  1 .ko (msm_kgsl.ko)
mm-drivers         →  3 .ko (msm_hw_fence, msm_hfi_core, msm_ext_display)
mmrm-driver        →  1 .ko (msm_mmrm.ko)
securemsm-kernel   →  5 .ko (smcinvoke, qcedev, qce50, qrng, hdcp_qseecom)
synx-kernel        →  1 .ko (synx_driver.ko)
video-driver       →  1 .ko (msm_video.ko)
```

### Cabeçalhos Reconstruídos por Engenharia Reversa (10)
```
kernel_platform/common/include/linux/mem-buf.h
kernel_platform/common/include/linux/msm_ion.h
kernel_platform/common/include/linux/msm_dma_iommu_mapping.h
kernel_platform/common/include/linux/qcom-iommu-util.h
kernel_platform/common/include/linux/hdcp_qseecom.h
kernel_platform/common/include/linux/qti-regmap-debugfs.h
kernel_platform/common/include/linux/soc/qcom/msm_mmrm.h
kernel_platform/common/include/linux/soc/qcom/battery_charger.h
kernel_platform/common/include/soc/qcom/minidump.h
kernel_platform/common/include/linux/pinctrl/qcom-pinctrl.h (modificado)
```

### Auditoria de Engenharia Reversa Fase 2 Completa
- **audio-kernel / swr_haptics_dlkm**: 8 funções comparadas 1:1 com Ghidra — 100% paridade
- Detalhes em: `vendor/qcom/opensource/audio-kernel/analysis.md`

### Touchscreen `zte_tpd` — Refatoração inicial concluída
- Branch de trabalho: `codex/zte-tpd-input-device-refactor`
- Commit atual: `ed8d40445d3ad748b0401221fb452ad335e14761`
- Arquivo principal corrigido:
  `kernel_platform/common/drivers/soc/qcom/zte/zte_tpd/syna_dev_set_up_input_device.c`
- Status:
  - `device + 656`, `device + 712` e writes diretos em bitmaps foram removidos do caminho compilado.
  - Setup de `struct input_dev` agora usa campos nomeados e helpers do input subsystem.
  - `./super_build.sh` compilou com sucesso.
  - `./repack_perfect_sign.sh` gerou imagem de kernel testável.

### Recovery Fastboot Test — Pronto para teste físico
- Backup ROM usado:
  `/mnt/e/Android/RM-11-Pro/BOOT/02-UL-Rom-16/images`
- Manifest:
  `/home/richtofen/android/output/recovery/RM11_RECOVERY_TEST_MANIFEST.md`
- Imagem recomendada para o primeiro teste:
  `/home/richtofen/android/output/recovery/rm11-e-rom-recovery-fastboot-fixed-kernel-bootbase.img`
- SHA-256:
  `da0cc68e4d814927d8232dadafd27a4c0741b8e547f2f457e1325113cf15788f`
- Comando:
  ```bash
  fastboot boot /home/richtofen/android/output/recovery/rm11-e-rom-recovery-fastboot-fixed-kernel-bootbase.img
  ```
- Regra: usar apenas `fastboot boot` neste estágio. Não fazer flash permanente.

### Comando de Compilação (referência)
```bash
# Execute a partir da raiz do repositório
PATH="$(pwd)/clang-r536225/bin:$PATH" \
KBUILD_MODPOST_WARN=1 make -C vendor/qcom/opensource/<TECHPACK> \
  KERNEL_SRC="$(pwd)/kernel_platform/common" \
  ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- LLVM=1 LLVM_IAS=1 \
  M="$(pwd)/vendor/qcom/opensource/<TECHPACK>" \
  AUDIO_ROOT="$(pwd)/vendor/qcom/opensource/<TECHPACK>" \
  -j$(nproc)
```

---

## TAREFA 1: Compilar bt-kernel (Bluetooth) ✅

**Source:** `vendor/qcom/opensource/bt-kernel/` — Makefile e Kbuild presentes  
**Módulos esperados:** `btpower.ko`, `btfmcodec.ko`, `bt_fm_swr.ko`  
**Decompiled disponível:** `decompiled/btpower/`, `decompiled/btfmcodec/`, `decompiled/bt_fm_swr/`

### Passos:
1. Verificar o Makefile e adaptar paths se necessário
2. Compilar com o comando padrão
3. Resolver headers/símbolos faltantes (provavelmente `msm_gpio_mpm_wake_set` já resolvido)
4. Validar símbolos contra `/proc/kallsyms`
5. Criar `analysis.md` local

---

## TAREFA 2: Validar touchscreen `zte_tpd` em recovery ⏳

**Source ativo:** `kernel_platform/common/drivers/soc/qcom/zte/zte_tpd/`
**Backup antes da refatoração:** `/home/richtofen/android/output/kernel/touchscreen-work/zte_tpd-before-refactor`

### Passos:
1. Bootar temporariamente a imagem de recovery gerada:
   ```bash
   fastboot boot /home/richtofen/android/output/recovery/rm11-e-rom-recovery-fastboot-fixed-kernel-bootbase.img
   ```
2. Verificar se recovery inicia e se o touchscreen responde.
3. Se recovery iniciar sem touch, coletar logs e procurar por `zte_tpd`, `syna`, `input`, GPIO, regulator e panel notifier.
4. Se houver reboot/crashdump, voltar ao kernel stock e coletar ramoops:
   ```bash
   cd /home/richtofen/android-kernel/android_kernel_nubia_sm8850_qwjujube
   ./scratch/get_ramoops.sh
   ```
5. Só depois de um boot temporário bem-sucedido começar a adaptação real da ramdisk de custom recovery.

---

## TAREFA 3: Compilar dataipa (Rede IPA) ⬜

**Source:** `vendor/qcom/opensource/dataipa/drivers/`  
**Módulos esperados:** `ipam.ko`, `ipanetm.ko`, `gsim.ko`, etc.  
**Decompiled disponível:** `decompiled/ipam/`, `decompiled/ipanetm/`, `decompiled/gsim/`

### Passos:
1. Verificar estrutura de build (Makefile/Kbuild)
2. Compilar
3. Resolver dependências (provavelmente precisa de headers de rede rmnet)
4. Validar símbolos

---

## TAREFA 4: Engenharia Reversa dos 12 Módulos ZTE ⬜

Módulos proprietários sem source que rodam no device. Código decompilado disponível em `decompiled/`.

| # | Módulo | Decompiled | Prioridade |
|---|--------|-----------|------------|
| 1 | `zte_charger_policy.ko` | `decompiled/zte_charger_policy/` | Alta — carregamento |
| 2 | `zte_power_supply.ko` | `decompiled/zte_power_supply/` | Alta — energia |
| 3 | `zte_led.ko` | `decompiled/zte_led/` | Média — LEDs |
| 4 | `zte_fingerprint.ko` | `decompiled/zte_fingerprint/` | Média |
| 5 | **[CONCLUÍDO]** `zte_misc.ko` | `decompiled/zte_misc/` | Média |
| 6 | **[CONCLUÍDO]** `zte_ir.ko` | `decompiled/zte_ir/` | Baixa — IR blaster |
| 7 | `zte_tpd.ko` | `decompiled/zte_tpd/` | Baixa |
| 8 | **[CONCLUÍDO]** `zte_imem_info.ko` | `decompiled/zte_imem_info/` | Baixa |
| 9 | **[CONCLUÍDO]** `zte_sensor_sensitivity.ko` | `decompiled/zte_sensor_sensitivity/` | Baixa |
| 10 | **[CONCLUÍDO]** `zte_stats_info.ko` | `decompiled/zte_stats_info/` | Baixa |
| 11 | **[CONCLUÍDO]** `zte_reboot_ext.ko` | `decompiled/zte_reboot_ext/` | Baixa |
| 12 | `zte_ramdisk_reboot.ko` | `decompiled/zte_ramdisk_reboot/` | Baixa |

### Abordagem:
- Fase 1: `nm -u` / `readelf -s` nos .ko originais do device para mapear símbolos
- Fase 2: Descompilação Ghidra função por função
- Decisão: Reescrever em C limpo ou usar os .ko binários diretamente

---

## TAREFA 5: GPU Overclock 1200MHz ⬜

**Objetivo:** Overclock estável da Adreno 830 para 1200MHz+

### Passos:
1. Extrair DTB da ROM oficial e descompilar para DTS
2. Localizar o nó `qcom,adreno` e as tabelas de OPP (Operating Performance Points)
3. Comparar com as frequências da pasta `decompiled/msm_kgsl/` e `decompiled/governor_msm_adreno_tz/`
4. Adicionar bins de 1200MHz com voltagem correta (extraída de `qcom,gpu-freq-table`)
5. Verificar limites térmicos em `decompiled/qti_thermal_vendor_hooks/`
6. Compilar novo DTB e testar via `fastboot boot`
7. Validar via `cat /sys/class/kgsl/kgsl-3d0/gpuclk` e benchmark

---

## TAREFA 6: KernelSU-Next ⬜

**Objetivo:** Integrar KernelSU-Next nativamente no kernel compilado

### Passos:
1. Clonar KernelSU-Next e aplicar patches no source do kernel
2. Adicionar configs necessários ao defconfig
3. Recompilar kernel GKI com suporte KernelSU
4. Testar boot com `fastboot boot`
5. Remover travas proprietárias ZTE (verificar `zte_misc.ko` e hooks de verificação)

---

## TAREFA 7: Compilação do Kernel Principal ⬜

**Bloqueio:** Depende do `canoe.fragment` (solicitado à ZTE)

### Quando disponível:
1. Aplicar fragment ao `gki_defconfig`
2. Compilar kernel + ~194 módulos de plataforma
3. Gerar boot.img com DTBs corretos
4. Testar boot completo
5. Substituir módulos da ROM pelos compilados

---

## TAREFA 8: WLAN ⬜

**Bloqueio:** Código fonte ausente (solicitado à ZTE)

### Alternativa:
- Buscar o driver `qca_cld3_peach_v2` em repositórios open-source Qualcomm/CodeLinaro
- Ou usar os .ko binários da ROM diretamente

---

## Referências Rápidas

| Arquivo | Descrição |
|---------|-----------|
| `REVERSE_ENGINEERING_MASTER_PLAN.md` | Memória global do projeto |
| `zte_missing_files_report.md` | Relatório formal para enviar à ZTE |
| `vendor/qcom/opensource/audio-kernel/analysis.md` | Documentação local do audio-kernel |
| `decompiled/` | Código descompilado (Ghidra) de todos os .ko da ROM |
| `kernel_platform/common/drivers/soc/qcom/zte_parity.c` | Bridge de símbolos GKI |

### Acesso ao Device
```bash
adb shell su -c "cat /proc/kallsyms"       # Símbolos do kernel
adb shell su -c "cat /proc/modules"         # Módulos carregados
adb shell su -c "uname -a"                  # Versão do kernel
```

### Script de Validação de Símbolos
```bash
python3 ~/.gemini/antigravity/brain/b4fcbd75-b4ec-4802-ab58-b97529611d28/scratch/check_ko_symbols.py
```
