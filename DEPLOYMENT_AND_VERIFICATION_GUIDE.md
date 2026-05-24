# DEPLOYMENT_AND_VERIFICATION_GUIDE — Protocolo de Implantação e Auditoria de Módulos (NX809J)

Este guia estabelece o padrão obrigatório de qualidade e segurança para testes, implantação e validação dinâmica dos drivers customizados/reconstruídos do Red Magic 11 Pro (NX809J) sob o GKI (Android 16 / Kernel 6.12).

---

## Current Project Synopsis

The RM11 Pro / NX809J recovery lane is now validated for recovery-partition-only development.

Current recovery facts:

- ABL unlock/restoration enabled working fastboot commands.
- `fastboot boot` is not a valid recovery-mode validation path on this device.
- Stock recovery boots through `adb reboot recovery`.
- Stock recovery display and touch work.
- Recovery ADB remains unauthorized, matching stock recovery behavior.
- `recovery_a` and `recovery_b` can be flashed safely.
- Byte-identical repacked stock recovery flashed to both recovery slots and still booted Android/recovery.
- marker-001 and marker-002 proved static ramdisk modifications are safe so far.
- marker-003 and marker-004 proved modified recovery ramdisk PNG resources execute on-device.

Current safe lane:

- use `recovery_a` and `recovery_b` only for recovery validation
- do not use `fastboot boot` for recovery validation
- keep rollback ready: `C:\RM11-test\recovery\rm11-repacked-stock-recovery.img`

Hard restrictions:

- no forced recovery cmdline images
- no `boot`, `vendor_boot`, `init_boot`, or `vbmeta` work
- no wipe/data behavior changes
- no services/init actions unless separately reviewed

---

## 1. O Problema do Carregamento Precoce (Early Boot Loading)

No sistema Android moderno, os drivers essenciais de hardware (Touch Screen `zte_tpd`, Charger Policy `zte_charger_policy`, reguladores de voltagem, etc.) são carregados pelo processo `init` nos estágios iniciais de inicialização do sistema (`early-init` / `init`).

**Por que a montagem de overlay simples (Magisk/KernelSU `post-fs-data`) falha para esses drivers?**
* O script `post-fs-data.sh` realiza o bind-mount substituindo os arquivos `.ko` na pasta física `/vendor_dlkm/lib/modules/` apenas depois que a partição `/data` é montada e descriptografada.
* Embora os arquivos em disco apareçam modificados após a descriptografia, o kernel **já carregou os drivers oficiais na memória RAM**. O sistema de arquivos é atualizado, mas o Kernel continua executando o código oficial antigo.

---

## 2. Protocolo de Auditoria: Como ter Certeza Absoluta de qual Driver está Ativo

Para garantir que o driver customizado é realmente o que está rodando, e não o oficial ocultado na memória, use os métodos abaixo.

### Método A: Comparação de Assinatura do compilador (GNU Build-ID) — O Método Mais Seguro
O compilador e o linker geram um hash de compilação único de 20 bytes para cada binário gerado na seção `.note.gnu.build-id`.

1. **Obtenha o Build-ID do driver compilado localmente na máquina host:**
   ```bash
   readelf -n vendor/zte/zte_tpd/zte_tpd.ko
   ```
   *Exemplo de saída:* `e206ab048242d44b95c5986dd2586713a66f70f7`

2. **Obtenha o Build-ID do driver oficial de backup:**
   ```bash
   readelf -n vendor/zte/zte_tpd/official_zte_tpd.ko
   ```
   *Exemplo de saída:* `716cee59c3e115942c168c240a27ed7db50b38f0`

3. **Consulte a assinatura do módulo carregado na RAM do smartphone:**
   Acesse a nota do ELF lida pelo kernel em tempo de execução usando privilégios de root:
   ```bash
   adb shell "su -c od -tx1 /sys/module/<nome_do_modulo>/notes/.note.gnu.build-id"
   ```
   * Se os bytes do output coincidirem com a assinatura do driver compilado, o seu driver está rodando.
   * Se coincidirem com os bytes do oficial, o driver oficial ainda está ativo na memória RAM.

---

### Método B: Comparação do Tamanho de Alocação de Memória
Drivers customizados possuem modificações em relação ao código da ZTE e são compilados sob diferentes níveis de otimização, resultando em tamanhos de seção diferentes.

1. **Compare o tamanho do código compilado e estático (texto e dados) localmente:**
   ```bash
   size zte_tpd.ko official_zte_tpd.ko
   ```
   *Exemplo de saída:*
   * Oficial: `text: 252002`, `data: 8077` -> Total: `262496` bytes
   * Customizado: `text: 228310`, `data: 211947` -> Total: `450058` bytes

2. **Consulte o consumo real de memória do módulo ativo no celular:**
   ```bash
   adb shell "cat /proc/modules | grep <nome_do_modulo>"
   ```
   Compare o número da coluna de bytes em memória (primeira coluna após o nome) com a proporção de alocação de seções para confirmar a correspondência.

---

### Método C: Inserção de Identificadores de Compilação no Código (Marcador Visual)
Sempre que for necessário um diagnóstico rápido, insira um parâmetro global visível via sysfs ou crie um nó específico.

1. **Adicione um parâmetro no arquivo `.c` do driver:**
   ```c
   static int custom_driver_active = 1;
   module_param(custom_driver_active, int, 0444);
   MODULE_PARM_DESC(custom_driver_active, "Indicates custom rebuilt driver is active");
   ```
2. **Consulte diretamente no smartphone após o boot:**
   ```bash
   adb shell "cat /sys/module/<nome_do_modulo>/parameters/custom_driver_active"
   ```
   * Se o arquivo existir e retornar `1`, o seu driver está rodando.
   * Se o arquivo não existir (ou o diretório de parâmetros estiver ausente), o driver rodando é o oficial da ZTE.

---

## 3. Estratégias de Implantação Segura

Dependendo de qual estágio de desenvolvimento e qual driver está sendo testado, adote a estratégia de implantação adequada descrita na tabela a seguir:

| Estratégia | Tipo de Driver | Risco | Reversibilidade | Recomendação |
| :--- | :--- | :--- | :--- | :--- |
| **1. Built-in (Image + Fastboot)** | Críticos (Touch, Power) | **Zero** | Instantânea (Hard Reboot) | **Ideal para depuração e testes locais** |
| **2. Flash de Partição (vendor_dlkm)** | Todos | **Médio** | Flash de stock DLKM | **Ideal para Builds de liberação / Paridade Binária** |
| **3. Hot-Reload (rmmod + insmod)** | Secundários (LEDs, IR) | **Alto** | Reinício Simples | **Não recomendado para drivers integrados a displays** |

---

### Estratégia 1: Compilação Estática (Built-in) e `fastboot boot` (Método Mais Seguro)
Esta é a melhor prática para ciclos rápidos de desenvolvimento e validação sem risco de inutilização temporária (brick) do hardware.

1. **Configuração da Compilação:**
   No arquivo `Kbuild` ou `Makefile` do driver a ser testado, altere a flag de montagem dinâmica para compilação embutida:
   ```diff
   - obj-m += zte_tpd.o
   + obj-y += zte_tpd.o
   ```
   Garanta que a pasta do driver esteja listada no makefile pai do kernel.

2. **Geração do Kernel Embutido:**
   Execute a compilação do kernel principal. O linker unirá o código do driver ao arquivo `Image`.

3. **Geração da Imagem de Boot:**
   Empacote o arquivo `Image` compilado em uma imagem de boot customizada de depuração (`boot.img` ou `vendor_boot.img`).

4. **Carregamento Temporário:**
   Boote o sistema diretamente na memória RAM usando o fastboot:
   ```bash
   fastboot boot boot.img
   ```
   * O kernel e seus drivers estáticos iniciarão diretamente na memória RAM.
   * Quando o Android carregar as partições do celular, as requisições de inicialização dos drivers em `/vendor_dlkm` serão ignoradas pelo kernel, pois a estrutura de controle já estará rodando na RAM.
   * **Se o sistema falhar/travar:** Pressione e segure o botão *Power* por 10 segundos. O smartphone reiniciará usando as partições gravadas fisicamente originais, eliminando riscos.

---

### Estratégia 1B: Recovery Temporário com Kernel Built-in
Use esta variação quando o objetivo for testar uma ramdisk de recovery ou preparar custom recovery sem gravar partições.

No NX809J, o `recovery.img` stock é uma imagem de recovery com ramdisk, mas sem kernel próprio:

```text
HEADER_VER      [4]
KERNEL_SZ       [0]
RAMDISK_SZ      [20458914]
RAMDISK_FMT     [lz4_legacy]
```

Por isso, o primeiro teste seguro usa o `boot.img` stock como base de header/kernel container, injeta o `Image` rebuildado + `dtb.img`, e substitui a ramdisk pela ramdisk do `recovery.img`.

Artifact atual recomendado:

```text
/home/richtofen/android/output/recovery/rm11-e-rom-recovery-fastboot-fixed-kernel-bootbase.img
SHA-256: da0cc68e4d814927d8232dadafd27a4c0741b8e547f2f457e1325113cf15788f
```

Base ROM usada:

```text
/mnt/e/Android/RM-11-Pro/BOOT/02-UL-Rom-16/images
```

Comando de teste:

```bash
fastboot boot /home/richtofen/android/output/recovery/rm11-e-rom-recovery-fastboot-fixed-kernel-bootbase.img
```

Critérios de decisão:

| Resultado | Próxima ação |
| :--- | :--- |
| Recovery inicia e touch funciona | Prosseguir para custom recovery ramdisk usando este kernel como base |
| Recovery inicia sem touch | Coletar logs e continuar reparo de `zte_tpd` |
| CrashDump/reboot | Voltar ao boot stock e coletar `scratch/ramoops.log` |

Regra obrigatória: esta imagem é somente para `fastboot boot`. Não usar `fastboot flash` neste estágio.

#### Resultado físico: `fastboot boot` não seleciona recovery

O teste físico mostrou que a imagem temporária com kernel/DTB customizados + ramdisk stock de recovery inicia Android, não recovery. Um teste de isolamento com kernel stock + DTB stock + ramdisk stock de recovery também iniciou Android:

```text
C:\RM11-test\recovery\rm11-stock-kernel-recovery-ramdisk-fastboot.img
SHA-256: ebe6bb82c4c6ab90bacb7b54cceb9486aaf25d69dd54657ceeda76aebbda5d06
```

Conclusão: neste aparelho, `fastboot boot` valida que a imagem é carregável pelo bootloader, mas não reproduz o handoff de recovery. A seleção real de recovery depende do caminho `adb reboot recovery` / bootloader recovery, BCB em `misc`, bootconfig e/ou comportamento de `init_boot`/`vendor_boot`.

Nova regra: não criar mais imagens com flags aleatórias `androidboot.*=recovery`. Uma tentativa com `androidboot.mode=recovery` e `androidboot.bootmode=recovery` causou CrashDump.

#### Próximo protocolo seguro: partição recovery

O próximo passo controlado está documentado em:

```text
RECOVERY_PARTITION_TEST_PLAN.md
```

O primeiro teste de partição deve ser um no-op: gravar somente `recovery_a` e `recovery_b` com a imagem repackada que é byte-idêntica ao stock `recovery.img`.

Baseline:

```text
C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
SHA-256: 1158594EB464748CD5E9313CC10B6505CDD58FE03CE09A9E7DA0B3F7A1E4187D
```

Comandos candidatos, apenas após todos os preflights do plano:

```powershell
fastboot flash recovery_a C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
fastboot flash recovery_b C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
```

Não tocar em `boot`, `vendor_boot`, `init_boot`, `vbmeta` ou metadata de slot ativo neste protocolo.

---

### Estratégia 2: Flash Físico na partição de Módulos (`vendor_dlkm`)
Quando o driver estiver validado e for necessário realizar testes de longa duração (benchmarks, estabilidade de overclock da GPU, KernelSU-Next).

1. Copie o driver patched com as assinaturas oficiais (`patch_tpd.py`) para a estrutura de diretórios do build de partições.
2. Gere a imagem do `vendor_dlkm.img` usando as ferramentas de build de plataforma (`mkfs.ext4` / `make_f2fs`).
3. Entre no modo bootloader (`fastboot`) e flasha a partição:
   ```bash
   fastboot flash vendor_dlkm vendor_dlkm.img
   ```

---

### Estratégia 3: Hot-Reloading (`rmmod` / `insmod`)
Utilize esta estratégia apenas se o driver puder ser removido sem acionar dependências críticas em cadeia.

* **Onde é seguro:** Drivers isolados que não fazem callbacks cruzados com o painel DRM (`msm_drm`), como o driver infravermelho (`zte_ir`) ou o driver de LEDs (`zte_led`).
* **Procedimento:**
  ```bash
  adb shell su -c "rmmod zte_led"
  adb shell su -c "insmod /data/adb/modules/zte_charger_policy_custom/vendor_dlkm/lib/modules/zte_led.ko"
  ```
* **Aviso de Perigo:** Nunca execute `rmmod zte_tpd` ou `rmmod zte_charger_policy` em runtime. A desconexão brusca provocará desreferenciamento de ponteiro nulo no loop de eventos do `panel_event_notifier`, causando interrupção instantânea do watchdog e reinicialização forçada em modo **"Qualcomm CrashDump Mode"**.

---

## 4. Regras Obrigatórias para Próximas Tarefas

1. **Nunca assuma que a presença do bind-mount prova que o código está rodando.** Sempre verifique o Build-ID e compare-o contra as assinaturas local e oficial.
2. **Documente os testes dinâmicos** criando um arquivo local `analysis.md` em cada diretório de driver, registrando o Build-ID ativo testado e a estratégia usada.
3. **Mantenha os backups oficiais:** Sempre preserve um binário de referência oficial (ex: `official_zte_tpd.ko`) para extração de assinaturas e restauração rápida de partições.
