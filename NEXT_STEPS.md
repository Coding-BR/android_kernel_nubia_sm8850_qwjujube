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

## Local ABL Unlock Artifact

A local ABL unlock research artifact is stored outside the git tree:

`~/android/output/abl/abl_unlock.elf`

This is intentionally not committed to the kernel repository. Keep it separate from:
- safetyguard PR
- zte_tpd touchscreen PR
- recovery fastboot test images

Do not flash blindly. Use only after confirming the exact ABL / partition workflow.

## Fastboot Boot Retest Result

After reflashing the local ABL unlock artifact and restoring the device to safe slot A state, `fastboot boot` behavior changed.

Previous result:
- `fastboot boot` failed with `remote: unknown command`

Current result:
- `fastboot boot C:\RM11-test\recovery\rm11-e-rom-recovery-fastboot-fixed-kernel-bootbase.img` sends and boots the image successfully
- the device returns through ADB as Android `device`, not `recovery`
- captured result shows `ro.boot.slot_suffix` as `_a`
- captured result shows `ro.bootmode` as `unknown`

Conclusion:
- fastboot temporary boot is now available
- the current recovery test image boots Android instead of recovery
- next work is recovery image construction/routing: bootconfig, cmdline, init_boot, vendor_boot, and recovery ramdisk behavior
- no further slot-B flashing is needed for this specific loop while `fastboot boot` works

## Force Recovery Cmdline Test Result

Temporary fastboot test only; no flashing.

Test image:
- force-recovery-mode.img
- based on working fastboot boot image
- kept stock recovery ramdisk
- added recovery-oriented cmdline flags:
  - androidboot.mode=recovery
  - androidboot.bootmode=recovery

Observed result:
- device entered blue memory dump / crashdump
- device recovered safely back to Android on slot A
- dmesg and pstore were not readable without root
- logcat/getprop captures were saved under:
  C:\RM11-test\force-recovery-crashdump

Conclusion:
- Do not retest force-recovery-mode.img.
- Do not continue random cmdline forcing.
- Stock recovery path works through adb reboot recovery, but fastboot boot recovery routing is still unresolved.
- Next work should inspect bootloader/recovery routing, vendor_boot/init_boot behavior, and captured logs before building another image.

## Next Isolation Test: Stock Kernel + Recovery Ramdisk

Purpose:
- determine whether the fastboot-booted recovery ramdisk returns to Android even with the stock kernel and stock DTB.
- separate recovery routing behavior from rebuilt-kernel behavior.

Image built:

```text
/home/richtofen/android/output/recovery/rm11-stock-kernel-recovery-ramdisk-fastboot.img
C:\RM11-test\recovery\rm11-stock-kernel-recovery-ramdisk-fastboot.img
SHA-256: ebe6bb82c4c6ab90bacb7b54cceb9486aaf25d69dd54657ceeda76aebbda5d06
```

Composition:
- E: ROM `boot.img` header/base
- E: ROM `boot.img` stock kernel
- E: ROM `boot.img` stock kernel DTB
- E: ROM `recovery.img` stock recovery ramdisk

Header check:

```text
HEADER_VER      [4]
KERNEL_SZ       [39819776]
RAMDISK_SZ      [20458914]
PAGESIZE        [4096]
CMDLINE         []
KERNEL_DTB_SZ   [19286848]
KERNEL_FMT      [raw]
RAMDISK_FMT     [lz4_legacy]
VBMETA
```

Test command:

```bash
fastboot boot C:\RM11-test\recovery\rm11-stock-kernel-recovery-ramdisk-fastboot.img
```

Interpretation:
- Boots Android: recovery routing is not caused by the rebuilt kernel. Continue investigating bootloader/BCB/bootconfig/vendor_boot/init_boot routing.
- Boots recovery: rebuilt kernel/DTB changes first-stage recovery behavior. Compare kernel config, bootconfig handling, and built-in driver changes.
- CrashDump/reboot: stop testing this image and collect available logs.

Rule:
- use only `fastboot boot`.
- do not flash this image.

## Stock Kernel Isolation Test Result

Tested image:

```text
C:\RM11-test\recovery\rm11-stock-kernel-recovery-ramdisk-fastboot.img
```

Composition:
- stock E: ROM kernel
- stock E: ROM kernel DTB
- stock E: ROM recovery ramdisk

Observed result:
- `fastboot boot` succeeded at transport/bootloader level.
- device booted Android, not recovery.

Conclusion:
- Android-vs-recovery routing is not caused by the rebuilt kernel or rebuilt DTB.
- Even stock kernel/DTB with stock recovery ramdisk enters Android when launched through `fastboot boot`.
- Stock recovery only enters recovery through `adb reboot recovery` / bootloader recovery handoff.

Additional clue from Android-side capture:
- after `fastboot boot`, Android still reports stock on-disk `init_boot` and `vendor_boot` vbmeta digests.
- `/proc/cmdline` and `/proc/bootconfig` are permission denied without root.
- `misc` is visible as `/dev/block/by-name/misc -> /dev/block/sda3`, but raw BCB reads are not available without root/EDL.

Current technical model:
- `fastboot boot` supplies a temporary boot image but still leaves the device in normal boot context.
- normal boot context loads or honors the on-disk `init_boot` / `vendor_boot` / bootconfig path.
- the recovery ramdisk alone is insufficient unless the bootloader also performs the recovery handoff.

Next focus:
- BCB / `misc` recovery command behavior
- bootloader recovery handoff
- `init_boot` and `vendor_boot` first-stage behavior
- whether custom recovery testing must move from `fastboot boot` to controlled recovery-partition replacement with stock-image restore ready

Avoid:
- more random `androidboot.*=recovery` cmdline images
- slot-B boot flashing for this loop
- any wipe/data-affecting recovery menu action

## Recovery Partition Baseline Artifact

Prepared a no-change repack of stock `recovery.img` to validate tooling before any custom recovery ramdisk edits.

Image:

```text
/home/richtofen/android/output/recovery/rm11-repacked-stock-recovery.img
C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
SHA-256: 1158594eb464748cd5e9313cc10b6505cdd58fe03ce09a9e7da0b3f7a1e4187d
```

Result:
- byte-identical to E: ROM stock `recovery.img`
- header remains ramdisk-only:

```text
HEADER_VER      [4]
KERNEL_SZ       [0]
RAMDISK_SZ      [20458914]
PAGESIZE        [4096]
CMDLINE         []
RAMDISK_FMT     [lz4_legacy]
VBMETA
```

Meaning:
- `magiskboot` unpack/repack preserves this recovery image exactly.
- The next recovery-partition test, when intentionally chosen, should start from this known-good baseline before adding custom recovery changes.
- This artifact is safe as a reference copy, but still should not be flashed casually.

## Controlled Recovery Partition Plan

Detailed plan:

```text
RECOVERY_PARTITION_TEST_PLAN.md
```

Next action is not another `fastboot boot` image. The next controlled gate is a no-op recovery partition write using the byte-identical repacked stock recovery baseline:

```text
C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
SHA-256: 1158594EB464748CD5E9313CC10B6505CDD58FE03CE09A9E7DA0B3F7A1E4187D
```

Candidate commands, only after all preflight checks in the plan pass:

```powershell
fastboot flash recovery_a C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
fastboot flash recovery_b C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
```

Do not touch boot, vendor_boot, init_boot, vbmeta, or active slot metadata for this validation.

## Minimal Recovery Marker Flash Result

Built and flashed a marker-only recovery ramdisk image.

Image:

```text
C:\RM11-test\recovery\rm11-recovery-marker-ramdisk.img
SHA-256: FF1DE80E20EF2CBEA3391351D6F184EBF2023DE30888B1D524F3854D57C01367
```

Only ramdisk addition:

```text
rm11_recovery_marker.txt
```

Flashed only:

```powershell
fastboot flash recovery_a C:\RM11-test\recovery\rm11-recovery-marker-ramdisk.img
fastboot flash recovery_b C:\RM11-test\recovery\rm11-recovery-marker-ramdisk.img
```

Result:
- `recovery_a` write OKAY
- `recovery_b` write OKAY
- Android rebooted safely afterward
- current slot remains `_a`
- `sys.boot_completed=1`
- `ro.boot.verifiedbootstate=orange`
- `ro.boot.flash.locked=0`

Next validation:

```powershell
adb reboot recovery
```

Manual checks:
- recovery UI appears
- display works
- touch works
- do not wipe data

## Marker 001 Recovery Flash Result

Built marker image:

```text
C:\RM11-test\recovery\rm11-recovery-marker-001.img
SHA-256: D14DA83E240888B711853E24196C60B647EE5166398F7E2FC27EDEECF535C61E
```

Marker path inside ramdisk:

```text
system/etc/rm11_recovery_marker.txt
```

Reason: stock recovery ramdisk has `/etc -> /system/etc`, so placing the marker under `system/etc` preserves the existing `/etc` symlink.

Marker content:

```text
RM11 recovery ramdisk marker test 001
```

Header:

```text
HEADER_VER      [4]
KERNEL_SZ       [0]
RAMDISK_SZ      [20458935]
RAMDISK_FMT     [lz4_legacy]
```

Flashed only:

```powershell
fastboot flash recovery_a C:\RM11-test\recovery\rm11-recovery-marker-001.img
fastboot flash recovery_b C:\RM11-test\recovery\rm11-recovery-marker-001.img
fastboot --set-active=a
```

Post-flash Android state:
- Android returned safely
- slot `_a`
- `sys.boot_completed=1`
- `ro.boot.verifiedbootstate=orange`
- `ro.boot.flash.locked=0`

Recovery boot command sent:

```powershell
adb reboot recovery
```

Manual validation result: PASS.
- recovery UI appeared
- display worked
- touch worked
- no CrashDump
- no FTM
- no black screen
- no wipe
- recovery ADB stayed unauthorized, matching stock recovery behavior

Runtime visibility:
- expected runtime marker path is `/etc/rm11_recovery_marker.txt`
- direct runtime marker verification is blocked because recovery ADB is unauthorized
- marker-001 still passes as a safe recovery partition and ramdisk modification test

## Marker 002 Recommendation

Do not build marker-002 as a behavior-changing image yet. The safest next marker should remain static:

- add `system/etc/rm11_recovery_marker_002.txt`
- optionally add one comment to `init.recovery.qcom.rc`
- do not add init actions
- do not add services
- do not change recovery properties
- do not change recovery UI resources yet
- do not modify boot, vendor_boot, init_boot, vbmeta, or slots
- do not use `fastboot boot` for recovery validation
- do not use any force-recovery cmdline image

Recommended marker-002 purpose:

```text
static ramdisk verification only
```

Reasoning:
- marker-001 already proved a modified recovery ramdisk can boot through the recovery partitions
- stock recovery ADB authorization is the blocker for reading the marker at runtime
- adding a log-writing action or property before solving visibility would add risk without guaranteeing useful evidence

Next useful investigation:
- determine whether stock recovery `View recovery logs` exposes enough detail for marker verification
- determine whether recovery ADB authorization can be made available without changing recovery behavior
- only after one of those works, consider a runtime marker under `/tmp` or another recovery-safe path

## Marker 002 Static Recovery Image

Built from:

```text
C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
SHA-256: 1158594EB464748CD5E9313CC10B6505CDD58FE03CE09A9E7DA0B3F7A1E4187D
```

Output:

```text
C:\RM11-test\recovery\rm11-recovery-marker-002.img
SHA-256: D8BF44E93C54EC61AB3D6810B01E21EE36E74A90CB0B458F7BBBFCF4928703C5
```

Static ramdisk changes:

```text
system/etc/rm11_recovery_marker_002.txt
init.recovery.qcom.rc comment only
```

Marker content:

```text
RM11 recovery ramdisk marker test 002
static file only
no init actions
no services
```

Header verification:

```text
HEADER_VER      [4]
KERNEL_SZ       [0]
RAMDISK_SZ      [20459092]
RAMDISK_FMT     [lz4_legacy]
```

Size:

```text
104857600 bytes = 0x6400000
```

Note: this is exactly the recovery partition size, matching the stock/repacked recovery image shape already validated on marker-001.

Flash commands:

```powershell
adb reboot bootloader
fastboot flash recovery_a C:\RM11-test\recovery\rm11-recovery-marker-002.img
fastboot flash recovery_b C:\RM11-test\recovery\rm11-recovery-marker-002.img
fastboot --set-active=a
fastboot reboot
```

Rollback:

```powershell
adb reboot bootloader
fastboot flash recovery_a C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
fastboot flash recovery_b C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
fastboot --set-active=a
fastboot reboot
```

Physical validation result: PASS.

- `recovery_a` write OKAY
- `recovery_b` write OKAY
- `fastboot --set-active=a` OKAY
- Android booted
- slot stayed `_a`
- `sys.boot_completed=1`
- `ro.boot.verifiedbootstate=orange`
- `ro.boot.flash.locked=0`
- `adb reboot recovery` reached recovery behavior
- recovery ADB stayed unauthorized, matching stock recovery behavior
- no CrashDump
- no FTM
- no black screen

Conclusion:
- marker-002 confirms static recovery ramdisk modification path is safe so far
- stop marker-only tests
- next step is the first minimal functional recovery change, still recovery-partition only

## Next Recovery Gate: First Functional Change

Rules:
- recovery partition only
- no boot, vendor_boot, init_boot, vbmeta, or slot metadata changes
- no `fastboot boot` recovery validation
- no force-recovery cmdline image
- no UI rewrite yet
- no service chains or long-running custom daemons
- no wipe/data-affecting changes
- keep rollback image ready: `C:\RM11-test\recovery\rm11-repacked-stock-recovery.img`

Recommended direction:
- inspect stock recovery init syntax and file contexts before building
- choose one tiny behavior change that can be observed or logged
- prefer volatile paths like `/tmp` if used
- do not change USB authorization behavior until the stock recovery ADB path is understood

Candidate to investigate, not yet approved for flashing:

```text
on init
    write /tmp/rm11_recovery_runtime_marker.txt "RM11 recovery runtime marker 003\n"
```

Build only after confirming the path and syntax are safe and after deciding how the runtime result will be observed.

## Marker 003 Recommendation: Visible Recovery Title

Inspection result:
- recovery menu labels are compiled into `system/bin/recovery`
- `View recovery logs` reads runtime logs such as `/tmp/recovery.log`, `/cache/recovery/log`, and `/cache/recovery/last_log`
- a static file or property marker is not visible while recovery ADB is unauthorized
- the recovery title is a PNG resource in the ramdisk
- current locale is `ro.product.locale=en-US`, so the English title resource is the smallest visible target

Recommended first functional change:

```text
res/images/recovery_en.png
```

Replace it with a same-size PNG that visibly reads:

```text
Recovery 003
```

Keep unchanged:
- boot
- vendor_boot
- init_boot
- vbmeta
- active slot metadata
- recovery binary
- menu strings
- init actions
- services
- properties
- wipe/data behavior

Why this is the safest first functional test:
- it is recovery partition only
- it is ramdisk content only
- it is visible from the recovery UI without authorized ADB
- it proves the modified recovery ramdisk resource is being loaded
- rollback is the known-good stock/repacked recovery image

Pass criteria:
- Android boots after flashing recovery partitions
- slot remains `_a`
- `sys.boot_completed=1`
- `adb reboot recovery` reaches recovery
- recovery title reads `Recovery 003`
- display/touch still work
- no CrashDump
- no FTM
- no black screen
- no wipe

Rollback:

```powershell
adb reboot bootloader
fastboot flash recovery_a C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
fastboot flash recovery_b C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
fastboot --set-active=a
fastboot reboot
```
