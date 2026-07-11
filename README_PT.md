# RedMagic 11 Pro (NX809J) - Guia de Compilação e Inicialização do Kernel

Este guia explica como compilar, empacotar e inicializar o kernel customizado com suporte a **KernelSU-Next** e **Paridade Binária** via memória RAM (sem alterar permanentemente as partições do celular), além de definir as próximas etapas de desenvolvimento.

---

## 🔴 BLOQUEADOR ATUAL: Travamento de Boot no Logo RedMagic

> [!WARNING]
> A **Build #8** inicializa além do early-init, mas **trava indefinidamente na tela do logotipo da RedMagic**.
> A causa raiz foi identificada como um **timeout de SCMI** no `arm-scmi.1.auto` que impede o carregamento do remoteproc ADSP/CDSP, fazendo com que o `keymint` e o `vold` não iniciem, bloqueando a descriptografia da partição `/data` (FBE).
>
> **Ajuda da comunidade é bem-vinda!** Se você tem experiência com SCMI da Qualcomm, GKI KCFI ou fluxo de boot do Android FBE, consulte os documentos de diagnóstico e as petições de conformidade abaixo.

| Documento | Idioma | Descrição |
|-----------|--------|-----------|
| **[BOOT_HANG_DIAGNOSTIC_PT.md](BOOT_HANG_DIAGNOSTIC_PT.md)** | 🇧🇷 Português | Diagnóstico completo do hang de boot com dados de ramoops, comparação stock vs custom, e guia de investigação |
| **[BOOT_HANG_DIAGNOSTIC_EN.md](BOOT_HANG_DIAGNOSTIC_EN.md)** | 🇺🇸 English | Full boot hang diagnostic with ramoops data, stock vs custom comparison, and investigation guide |
| **[NOTIFICACAO_COMPLIANCE_GPL_ZTE.md](NOTIFICACAO_COMPLIANCE_GPL_ZTE.md)** | 🇧🇷 Português | Notificação formal de descumprimento da licença GPL v2 e lista de fontes em falta para enviar à ZTE |
| **[GPL_COMPLIANCE_NOTICE_ZTE.md](GPL_COMPLIANCE_NOTICE_ZTE.md)** | 🇺🇸 English | Formal GPL v2 compliance notice and missing components report to send to ZTE support |

---

## 📋 Pré-requisitos

Para que a compilação e o empacotamento funcionem corretamente, você deve providenciar os seguintes componentes (que estão configurados no `.gitignore` para manter o repositório limpo):

1. **Toolchain Clang (r536225)**:
   - A revisão AOSP e o hash exatos do compilador estão fixados em `toolchains.lock.json`.
   - Reversa resolve a cópia padrão em `${XDG_CACHE_HOME:-$HOME/.cache}/reversa/toolchains/android-clang-linux-x86/clang-r536225`.
   - Defina `CLANG_DIR` somente para usar outro local; o resolvedor ainda valida a identidade e o hash fixados.

2. **Device Tree Blob (`dtb.img`)**:
   - Extraia o arquivo `dtb.img` oficial a partir da partição/imagem de boot (`boot.img`) de estoque da ZTE (versão userdebug correspondente).
   - Coloque o arquivo `dtb.img` diretamente na raiz deste repositório. Ele é necessário para concatenar os Device Trees à imagem do kernel durante o empacotamento.

3. **Dependências do Host**:
   - Certifique-se de ter instalado os pacotes essenciais de compilação do Linux (`build-essential`, `libssl-dev`, `bison`, `flex`, `libelf-dev`, `python3`, etc.).

---

## 🔓 Guia de Habilitação do Fastboot

Para habilitar o acesso e os comandos de gravação do Fastboot no RedMagic 11 Pro (NX809J) e outros dispositivos ZTE/Nubia, siga estes passos:

### 1. Instalar o Fastboot e o ADB
* **Windows**: Baixe o [Android SDK Platform-Tools](https://developer.android.com/tools/releases/platform-tools) oficial e adicione-o ao PATH do seu sistema.
* **Linux (Ubuntu/Debian)**: Instale via apt:
  ```bash
  sudo apt update
  sudo apt install android-tools-adb android-tools-fastboot
  ```
* **macOS**: Instale via Homebrew:
  ```bash
  brew install android-platform-tools
  ```

### 2. Gravar a ABL Desbloqueada via Modo EDL
Para habilitar o Fastboot, primeiro você precisa substituir a partição do gerenciador de boot (`abl`) oficial pela versão customizada. Como o Fastboot é bloqueado/desabilitado por padrão nas ROMs de estoque, **você ainda não tem acesso ao Fastboot**. Portanto, este processo deve ser feito exclusivamente em **Modo EDL**:
1. Inicialize seu dispositivo no modo EDL (Emergency Download Mode) e abra a ferramenta **ZTE Family Toolbox** (ZTE Toolbox).
2. Faça o backup/dump das suas partições oficiais **`abl_a`** e **`abl_b`** usando a ferramenta (mantenha esses backups salvos!).
3. Grave o arquivo de ABL desbloqueada fornecido neste repositório (🔴 **`abl_unlock.elf`**) em ambos os slots (`abl_a` e `abl_b`) selecionando a **Opção 12 (Gravar Partição / Write Partition)** no ZTE Family Toolbox.
4. > [!IMPORTANT]
   > **ZTE Toolbox Opção 19**: Imediatamente após gravar a ABL desbloqueada pelo ZTE Family Toolbox, você **DEVE** executar a **Opção 19** no ZTE Family Toolbox para limpar os flags e logs temporários de boot do aparelho. Se você pular este passo, o dispositivo entrará em **Dumper Mode** (tela de Crash Dump) na inicialização seguinte e não dará boot.
5. > [!WARNING]
   > **NÃO utilize a Opção 18 (Corrigir Digital/Fingerprint)** no ZTE Family Toolbox! Apesar de ser rotulada como correção do leitor de digitais, o uso da Opção 18 corrompe as configurações de inicialização, forçando o aparelho a cair novamente na tela de **Dumper Mode** (Crash Dump). Utilize apenas a Opção 19 para limpar os flags e evite totalmente a Opção 18.

### 3. Desativar a Verificação do vbmeta
Após gravar com sucesso a ABL desbloqueada e limpar os flags com a Opção 19, reinicie o aparelho. Agora o aparelho permitirá o acesso e comandos de gravação no **Modo Fastboot**:
1. Inicialize o dispositivo no modo Fastboot.
2. Você **DEVE** gravar as imagens de `vbmeta` e `vbmeta_system` de estoque desativando as proteções de integridade para evitar bootloops ou falhas de verificação do AVB:
   ```bash
   fastboot --disable-verity flash vbmeta_a vbmeta.img
   fastboot --disable-verity flash vbmeta_b vbmeta.img
   fastboot --disable-verity --disable-verification flash vbmeta_system_a vbmeta_system.img
   fastboot --disable-verity --disable-verification flash vbmeta_system_b vbmeta_system.img
   ```
   > [!CAUTION]
   > **NUNCA** use a flag `--disable-verification` ao gravar `vbmeta_a` ou `vbmeta_b` (use apenas `--disable-verity`). Se você utilizar `--disable-verification` no `vbmeta` principal, o seu dispositivo ficará travado no bootloader (stuck at bootloader) e não inicializará, exigindo uma recuperação completa via **Modo EDL**!
3. Por fim, caso queira efetuar o desbloqueio do bootloader, execute:
   ```bash
   fastboot flashing unlock
   ```
   Confirme o desbloqueio na tela do aparelho usando os botões de volume e o botão Power.

---

## 🚀 1. Como Compilar e Inicializar o Kernel

### Passo A: Compilar o Kernel principal e Techpacks
Temos o script unificado [super_build.sh](super_build.sh) que configura o ambiente, aplica a defconfig da plataforma (`nx809j_defconfig`), insere as flags de segurança (CFI Permissive, KernelSU-Next) e compila os binários com a toolchain Clang apropriada.

Execute no terminal:
```bash
./super_build.sh
```
* O kernel será compilado e a imagem resultante ficará localizada em: `kernel_platform/common/arch/arm64/boot/Image`.

---

### Passo B: Empacotar e Assinar a Imagem de Boot (DTB + AVB)
Os drivers dinâmicos Qualcomm exigem a junção física dos Device Trees (`dtb.img`) no cabeçalho do kernel. O script [repack_perfect_sign.sh](repack_perfect_sign.sh) cuida do empacotamento sem ramdisk interno (tamanho exato de 64MB) e faz a assinatura criptográfica obrigatória via AVB para evitar bloqueio do bootloader.

Execute:
```bash
./repack_perfect_sign.sh
```
* Isso gerará a imagem assinada pronta para boot na raiz do projeto: **`dev_reverse_perfect.img`**.

---

### Passo C: Inicializar na Memória RAM (Boot Temporário)

> [!IMPORTANT]
> **Compatibilidade do Compilador e AnyKernel:**
> Para rodar este kernel atual, o dispositivo **deve possuir previamente** um kernel funcional compilado com o **Clang 20** instalado.
> Portanto, antes de tentar fazer a inicialização temporária na memória RAM (`fastboot boot`), é necessário subir a imagem de boot no smartphone usando o **AnyKernel** para que os patches como o **SUSFS** e o **Linux Droidspaces** sejam devidamente ativados.
> Caso contrário, a inicialização temporária não será estável ou compatível para o desenvolvimento, uma vez que o kernel de fábrica original utiliza uma versão anterior do Clang.

> [!WARNING]
> **NUNCA execute `fastboot flash boot` ou grave esta imagem permanentemente no dispositivo durante a fase de testes.** O boot deve ser feito sempre temporariamente na RAM. Se o sistema travar ou apresentar falhas, um simples pressionar do botão Power por 10 segundos restaura o boot oficial original.

1. Reinicie o smartphone no modo bootloader:
   ```bash
   adb reboot bootloader
   ```
2. Carregue a imagem diretamente na memória RAM usando o fastboot:
   ```bash
   sudo fastboot boot dev_reverse_perfect.img
   ```
   *(Insira a senha de superusuário (sudo) caso o terminal solicite privilégios de root para comunicação USB com o fastboot)*.

3. O celular iniciará o Android. Após inicializado, você pode validar o kernel ativo rodando:
   ```bash
   adb shell uname -a
   ```
   Você verá a string indicando a sua compilação customizada com suporte ao **KernelSU-Next** ativo.

## 📦 2. Arquitetura de Drivers ZTE (obj-y vs obj-m)

No arquivo [drivers/soc/qcom/zte/Makefile](kernel_platform/common/drivers/soc/qcom/zte/Makefile), os drivers estão configurados assim:

```makefile
obj-y += zte_misc/
obj-y += zte_power_supply/
obj-y += zte_imem_info/
obj-y += zte_stats_info/
obj-y += zte_sensor_sensitivity/
obj-y += zte_ir/
obj-y += zte_reboot_ext/
obj-y += zte_ramdisk_reboot/
obj-y += zte_led/
obj-y += zte_fingerprint/
obj-y += zte_charger_policy/
obj-m += zte_tpd/
```

### O que significa `y` ou `m`?

*   **`obj-y` (Built-in / Estaticamente Embutido):**
    O código do driver é compilado diretamente para o binário executável principal do kernel (`vmlinux` / `Image`).
    *   **Como carrega:** É carregado na memória RAM como parte do processo de inicialização do kernel nas fases bem iniciais (early init).
    *   **Vantagens:** Não necessita de arquivos externos `.ko` no armazenamento. Inicia instantaneamente e ignora assinaturas ou restrições de verificação tardia de carregamento de módulos.

*   **`obj-m` (Modular / Módulo Carregável):**
    O código do driver é compilado como um binário dinâmico independente com extensão `.ko` (Kernel Object) (ex: `zte_tpd.ko`).
    *   **Como carrega:** O kernel principal inicia sem esse código. O sistema operacional (Android) o carrega em tempo de execução usando comandos como `insmod` ou `modprobe` (geralmente a partir de `/vendor_dlkm/lib/modules/` ou via scripts do KernelSU no `post-fs-data`).
    *   **Vantagens:** Permite a substituição dinâmica do driver sem recompilar o kernel inteiro, resolve colisões de símbolos no build time e economiza espaço estático no kernel central.

---

### Quais drivers são compilados open-source vs. dependências proprietárias do Smartphone?

Como este projeto foi desenhado para testes via `fastboot boot` (executando um kernel customizado temporariamente em RAM enquanto monta as partições físicas do próprio telefone), a pilha de drivers se divide em duas categorias:

#### 1. Drivers Compilados a partir do Código Aberto nesta Árvore:
*   **Touchscreen (`zte_tpd.ko`):** Reconstruído dinamicamente como módulo modular (`obj-m`), otimizado com alocação dinâmica oficial de plataforma e agendamentos unbound para evitar pânicos de CPU e watchdog.
    *   **Como é carregado e roda:** Como a partição `/vendor_dlkm` física original do celular é montada como somente leitura (Read-Only), o driver customizado `zte_tpd.ko` é carregado no aparelho usando o **KernelSU-Next**. O arquivo compilado é copiado para `/data/adb/modules/zte_tpd_only/vendor_dlkm/lib/modules/zte_tpd.ko`. Para garantir a montagem (overlay) correta do driver no Android 16 sem conflitos, criamos um script de hook em `/data/adb/modules/zte_tpd_only/post-fs-data.sh` contendo o comando de bind-mount manual (`mount -o bind /data/adb/modules/zte_tpd_only/vendor_dlkm/lib/modules/zte_tpd.ko /vendor_dlkm/lib/modules/zte_tpd.ko`). Durante a inicialização, o motor do KernelSU executa esse script e sobrepõe o nosso driver compilado por cima do original de fábrica de forma segura.
*   **Drivers Centrais ZTE:** Todos os 11 drivers listados sob `obj-y` (`zte_misc`, `zte_led`, `zte_fingerprint`, `zte_charger_policy`, etc.) são compilados e embutidos diretamente no executável `Image` estático.
*   **KernelSU-Next:** Integrado e embutido nativamente no kernel estático.
*   **Stubs de Compatibilidade (`zte_parity.c`):** Mantém a paridade de ABI expondo stubs para os símbolos exigidos pelos módulos proprietários da Qualcomm.

#### 2. Drivers Preexistentes que Dependem do Aparelho (Dependências da ROM):
Ao inicializar via RAM, o Android monta as partições físicas oficiais `/vendor` e `/vendor_dlkm` gravadas no armazenamento flash do celular.
Portanto, o dispositivo continua carregando os drivers Qualcomm de fábrica originais diretamente destas partições:
*   **Câmera e Display:** `camera.ko`, `msm_drm.ko`, `msm_video.ko`.
*   **Áudio e Energia:** Módulos de codec de áudio (`wcd939x_dlkm.ko`), feedback tátil (haptics) e gerenciamento de bateria da SoC.
*   **Conectividade:** Drivers de modem móvel (`dataipa.ko`) e driver de Wi-Fi (`qca_cld3_peach_v2.ko`).

*Nota: Graças ao fato de mantermos paridade binária e de ABI perfeita no nosso kernel customizado (preservando configurações ativas como CONFIG_SCHED_CLASS_EXT e CONFIG_DEBUG_INFO_BTF), esses módulos proprietários da ROM oficial carregam e rodam sem problemas ou crash dumps.*

---

## 🛠️ 3. Próximos Passos do Projeto

Now that we have achieved stable binary parity compatibility (where the custom kernel loads and runs the system normally), we can begin the specific development tasks described in [NEXT_STEPS.md](NEXT_STEPS.md):

### 1. Implantar e Testar os Módulos Reconstruídos (.ko)
Como o carregamento via RAM usa as partições físicas e monta `/vendor_dlkm` do sistema original, os módulos ativos no momento são os originais da ZTE. Para colocar em execução as nossas reconstruções (com código aberto refeito via Ghidra):
* **Método Built-in:** Modifique o `Kbuild` dos drivers que reconstruímos (como `zte_charger_policy` e `zte_led`) alterando a compilação para embutida (`obj-y` em vez de `obj-m`). Assim, eles serão compilados diretamente dentro do arquivo `Image` e carregados antes do sistema inicializar os oficiais.

### 2. Overclock da GPU para 1200MHz+
* Extrair e descompilar o Device Tree original (`dtb.img` ou `dtbo.img`) para formato texto (`.dts`).
* Localizar a tabela de OPPs (Operating Performance Points) da GPU Adreno 830.
* Adicionar os steps de frequência de overclock (com os binds de tensão corretos) para atingir 1200MHz+ de forma estável.
* Recompilar a árvore e testar via `fastboot boot`.

### 3. Integração Completa de Outros Techpacks
* Adaptar e compilar os módulos de toque (`touch-drivers`) para validar a interação física do painel usando nossa árvore.
* Validar a compilação de redes móveis e conectividade (`dataipa`).

### 4. Auditoria de Travas Proprietárias e CFI
* Monitorar os logs de pânico (`dmesg` e `console-ramoops-0`) para mapear qualquer falha de CFI (Control Flow Integrity) remanescente nos módulos carregados.

---

## 📬 Contato e Suporte

Se você tiver dúvidas, precisar de suporte ou quiser colaborar neste projeto de engenharia reversa, entre em contato através de:
* 📧 **E-mail:** [idealcreativesuporte@idealcreative.com.br](mailto:idealcreativesuporte@idealcreative.com.br)
* 💬 **Grupo do Telegram:** [Entrar no Grupo](https://t.me/redmagic11PR0) (Ou contato direto: [@idealcreative](https://t.me/idealcreative))
* 🌐 **Tópico no XDA:** [Guia de Unlock e Ferramentas do RedMagic 11 Pro](https://xdaforums.com/t/red-magic-11-pro-guide-bootloader-unlock-free-also-support-rm10-pad3pro-z70u-z80u-unlock-zte-family-toolbox.4780930/)
