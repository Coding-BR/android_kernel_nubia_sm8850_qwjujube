# RedMagic 11 Pro (NX809J) - Guia de Compilação e Inicialização do Kernel

Este guia explica como compilar, empacotar e inicializar o kernel customizado com suporte a **KernelSU-Next** e **Paridade Binária** via memória RAM (sem alterar permanentemente as partições do celular), além de definir as próximas etapas de desenvolvimento.

---

## 📋 Pré-requisitos

Para que a compilação e o empacotamento funcionem corretamente, você deve providenciar os seguintes componentes (que estão configurados no `.gitignore` para manter o repositório limpo):

1. **Toolchain Clang (r536225)**:
   - Baixe a versão do Clang do AOSP (revision `r536225`).
   - Extraia a toolchain na raiz deste repositório em uma pasta chamada `clang-r536225/` (de modo que o binário principal fique em `clang-r536225/bin/clang`).
   - Alternativamente, você pode definir a variável de ambiente `CLANG_DIR` apontando para o seu diretório do Clang antes de compilar.

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

---

## 🧪 2. Imagem de Teste para Recovery com Touchscreen

O trabalho atual de custom recovery **não** começa gravando `recovery.img`. No NX809J, o `recovery.img` stock é uma imagem Android boot com ramdisk, mas sem kernel próprio:

```text
HEADER_VER      [4]
KERNEL_SZ       [0]
RAMDISK_FMT     [lz4_legacy]
```

Para o primeiro teste seguro, use uma imagem baseada no header de `boot.img`, combinando:

* `boot.img` stock da ROM em E: como container de boot
* ramdisk do `recovery.img` stock da ROM em E:
* kernel `Image` recompilado
* `dtb.img` recompilado
* refatoração built-in do touchscreen `zte_tpd` da branch `codex/zte-tpd-input-device-refactor`

Artifact atual recomendado para o primeiro teste:

```text
/home/richtofen/android/output/recovery/rm11-e-rom-recovery-fastboot-fixed-kernel-bootbase.img
SHA-256: da0cc68e4d814927d8232dadafd27a4c0741b8e547f2f457e1325113cf15788f
```

Inicialize temporariamente:

```bash
fastboot boot /home/richtofen/android/output/recovery/rm11-e-rom-recovery-fastboot-fixed-kernel-bootbase.img
```

> [!WARNING]
> Não use `fastboot flash` para esta imagem. Ela é apenas um teste temporário.

Caminho de decisão:

| Resultado | Próxima ação |
| :--- | :--- |
| Recovery inicia e touch funciona | Começar a adaptação da ramdisk de custom recovery usando este kernel como base |
| Recovery inicia sem touch | Coletar logs e continuar o reparo de `zte_tpd` |
| CrashDump/reboot | Bootar stock novamente e coletar `scratch/ramoops.log` |

O manifest completo está salvo em:

```text
/home/richtofen/android/output/recovery/RM11_RECOVERY_TEST_MANIFEST.md
```

---

## 🛠️ 3. Próximos Passos do Projeto

Agora que alcançamos a compatibilidade de paridade binária estável (onde o kernel customizado carrega e roda o sistema normalmente), podemos iniciar as tarefas específicas de desenvolvimento descritas em [NEXT_STEPS.md](NEXT_STEPS.md):

### 1. Validar `zte_tpd` no Recovery
O caminho ativo do touchscreen neste aparelho é o driver ZTE/Synaptics reconstruído em:

```text
kernel_platform/common/drivers/soc/qcom/zte/zte_tpd/
```

A branch atual `codex/zte-tpd-input-device-refactor` corrige o risco óbvio de corrupção de `struct input_dev`, substituindo writes em offsets crus como `device + 656` e `device + 712` por campos nomeados e helpers oficiais do subsistema de input. A próxima prova é o teste físico usando a imagem de recovery acima.

### 2. Overclock da GPU para 1200MHz+
* Extrair e descompilar o Device Tree original (`dtb.img` ou `dtbo.img`) para formato texto (`.dts`).
* Localizar a tabela de OPPs (Operating Performance Points) da GPU Adreno 830.
* Adicionar os steps de frequência de overclock (com os binds de tensão corretos) para atingir 1200MHz+ de forma estável.
* Recompilar a árvore e testar via `fastboot boot`.

### 3. Integração Completa de Outros Techpacks
* Continuar a validação de `zte_tpd` a partir de logs reais do recovery, em vez de começar por módulos genéricos `goodix` ou `focaltech`.
* Validar a compilação de redes móveis e conectividade (`dataipa`).

### 4. Auditoria de Travas Proprietárias e CFI
* Monitorar os logs de pânico (`dmesg` e `console-ramoops-0`) para mapear qualquer falha de CFI (Control Flow Integrity) remanescente nos módulos carregados.

---

## 📬 Contato e Suporte

Se você tiver dúvidas, precisar de suporte ou quiser colaborar neste projeto de engenharia reversa, entre em contato através de:
* 📧 **E-mail:** [idealcreativesuporte@idealcreative.com.br](mailto:idealcreativesuporte@idealcreative.com.br)
* 💬 **Grupo do Telegram:** [Entrar no Grupo](https://web.telegram.org/a/#-1003471835008_307)
* 🌐 **Tópico no XDA:** [Guia de Unlock e Ferramentas do RedMagic 11 Pro](https://xdaforums.com/t/red-magic-11-pro-guide-bootloader-unlock-free-also-support-rm10-pad3pro-z70u-z80u-unlock-zte-family-toolbox.4780930/)
