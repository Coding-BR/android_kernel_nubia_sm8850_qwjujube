# NEXT_STEPS - Tarefas Pendentes para Continuação

**Dispositivo:** RedMagic 11 Pro (NX809J) — Snapdragon 8 Gen 5 (SM8850)  
**Última atualização:** 2026-05-19  

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

### Comando de Compilação (referência)
```bash
# Execute a partir da raiz do repositório
export CLANG_DIR="$(python3 scripts/toolchains/resolve_android_clang.py)"
PATH="$CLANG_DIR/bin:$PATH" \
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

## TAREFA 2: Compilar touch-drivers ⬜

**Source:** `vendor/qcom/opensource/touch-drivers/` — goodix_berlin_driver, focaltech_touch  
**Decompiled disponível:** Não verificado

### Passos:
1. Verificar qual driver de toque o NX809J usa (verificar DT ou `/proc/bus/input/`)
2. Compilar o driver correto
3. Validar símbolos

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

## TAREFA 4: Reconstrução e Validação dos 12 Módulos ZTE EM ANDAMENTO

O corpus de engenharia reversa cobre os 12 módulos. Código C reconstruído
também existe para todos eles, mas presença de source não equivale a uma
validação atual de compilação, ABI, KCFI, boot ou hardware. O estado de
proveniência autoritativo está em [SOURCE_PROVENANCE.md](SOURCE_PROVENANCE.md).

| # | Módulo | Evidência Ghidra | Reconstrução C | Estado atual |
|---|--------|------------------|----------------|--------------|
| 1 | `zte_charger_policy.ko` | `decompiled/zte_charger_policy/` | `vendor/qcom/opensource/zte-drivers/zte_charger_policy/` | Source presente; gate atual pendente |
| 2 | `zte_power_supply.ko` | `decompiled/zte_power_supply/` | `vendor/qcom/opensource/zte-drivers/zte_power_supply/` | Source presente; gate atual pendente |
| 3 | `zte_led.ko` | `decompiled/zte_led/` | `vendor/qcom/opensource/zte-drivers/zte_led/` | Source presente; gate atual pendente |
| 4 | `zte_fingerprint.ko` | `decompiled/zte_fingerprint/` | `vendor/qcom/opensource/zte-drivers/zte_fingerprint/` | Source presente; gate atual pendente |
| 5 | `zte_misc.ko` | `decompiled/zte_misc/` | `vendor/qcom/opensource/zte-drivers/zte_misc/` | Source presente; gate atual pendente |
| 6 | `zte_ir.ko` | `decompiled/zte_ir/` | `vendor/qcom/opensource/zte-drivers/zte_ir/` | Source presente; gate atual pendente |
| 7 | `zte_tpd.ko` | `decompiled/zte_tpd/` | `kernel_platform/common/drivers/soc/qcom/zte/zte_tpd/` | Build-target Ghidra; gate atual pendente |
| 8 | `zte_imem_info.ko` | `decompiled/zte_imem_info/` | `vendor/qcom/opensource/zte-drivers/zte_imem_info/` | Source presente; gate atual pendente |
| 9 | `zte_sensor_sensitivity.ko` | `decompiled/zte_sensor_sensitivity/` | `vendor/qcom/opensource/zte-drivers/zte_sensor_sensitivity/` | Source presente; gate atual pendente |
| 10 | `zte_stats_info.ko` | `decompiled/zte_stats_info/` | `vendor/qcom/opensource/zte-drivers/zte_stats_info/` | Source presente; gate atual pendente |
| 11 | `zte_reboot_ext.ko` | `decompiled/zte_reboot_ext/` | `vendor/qcom/opensource/zte-drivers/zte_reboot_ext/` | Source presente; gate atual pendente |
| 12 | `zte_ramdisk_reboot.ko` | `decompiled/zte_ramdisk_reboot/` | `vendor/qcom/opensource/zte-drivers/zte_ramdisk_reboot/` | Source presente; gate atual pendente |

### Abordagem:
- Fase 1: `nm -u` / `readelf -s` nos .ko originais do device para mapear símbolos
- Fase 2: Descompilação Ghidra função por função
- Fase 3: Compilar cada reconstrução e validar ABI/KCFI
- Fase 4: Validar boot, suspend/resume e comportamento no hardware
- Regra: módulos binários oficiais são evidência checksummed, não substitutos de source

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
