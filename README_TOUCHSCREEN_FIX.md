# Correção do Kernel GKI e Portabilidade do Driver Touchscreen ZTE (RedMagic 11 Pro - NX809J)

Este documento descreve as modificações, estratégias de engenharia reversa e soluções aplicadas no Kernel GKI (Android 16 / Linux 6.12) para resolver o travamento em **ZTE Memory Dump Mode** e restaurar as funções do subsistema de hardware.

---

## 1. Resumo das Correções de Estabilidade e Boot (Memory Dump Mode)

### A. Validação Global de Ponteiros no Formatador (`vsprintf.c`)
- **Problema:** Módulos stock proprietários da ROM (ex: `ipam.ko` para rede IPA) compilados com flags diferentes do GKI realizavam relocações absolutas de 64 bits truncadas para 32 bits (ex: `0xffffffc0ffff135e` -> `0x00000000ffff135e`). Ao logar strings com `%s`, o `vsprintf` tentava ler o ponteiro truncado, causando falha de segmentação no kernel (Data Abort) e bootloop em Dump Mode.
- **Solução:** Modificamos a função `check_pointer_msg` em `kernel_platform/common/lib/vsprintf.c` para invalidar com `(efault)` qualquer endereço abaixo do limite inferior do espaço virtual do kernel (`0xfff0000000000000UL` em `CONFIG_64BIT`), evitando o desreferenciamento inseguro.

### B. Hardenização das Funções de String em Assembly ARM64 (`strcmp`, `strlen`, `strncmp`)
- **Problema:** Os mesmos ponteiros truncados de 64 bits eram repassados a funções primitivas de string em assembly otimizado, como `strcmp` e `strlen`, gerando pânico imediato na desreferenciação (ex: na instrução `ldrb w3, [x1], #1` no `strcmp`).
- **Solução:** Modificamos os arquivos de biblioteca em assembly no núcleo do kernel ARM64:
  - **[strcmp.S](file:///home/adrianojr59/Vídeos/NX809J_Android16_kernel/kernel_platform/common/arch/arm64/lib/strcmp.S):** Injetamos uma validação de prefixo dos registradores `x0` e `x1`. Caso apontem para fora do kernel (shift right por 52 diferente de `0xfff` e não nulos), a comparação é abortada com segurança e retorna diferente (`ne`).
  - **[strlen.S](file:///home/adrianojr59/Vídeos/NX809J_Android16_kernel/kernel_platform/common/arch/arm64/lib/strlen.S):** Valida o registrador `x0`. Se inválido, retorna tamanho `0` imediatamente.
  - **[strncmp.S](file:///home/adrianojr59/Vídeos/NX809J_Android16_kernel/kernel_platform/common/arch/arm64/lib/strncmp.S):** Semelhante ao `strcmp.S`, protege `x0` e `x1`.

### C. Salvaguardas no Iterador do Device Core (`klist.c`)
- **Problema:** Falhas na inicialização e matching de periféricos faziam com que o kernel tentasse iterar sobre dispositivos filhos usando cabeças de listas (`klist`) não inicializadas (nulas), gerando desreferenciação de ponteiro nulo em `klist_next` / `klist_prev`.
- **Solução:** Injetamos verificações nulas de segurança nas funções do iterador `lib/klist.c` para gerar apenas um stack trace de alerta (`dump_stack`) e abortar a iteração sem derrubar o kernel.

---

## 2. Portabilidade e Compilação In-Tree do Driver Touchscreen (`zte_tpd`)

- **Bypass de insmod Dinâmico:** Modificamos o carregador de módulos em `kernel/module/main.c` para interceptar e retornar sucesso imediato para carregamentos do ramdisk que contenham `zte_` ou `panel_event_notifier`, forçando o uso dos drivers compilados embutidos no kernel.
- **Estruturação in-tree:** Movemos o driver descompilado para `kernel_platform/common/drivers/soc/qcom/zte/zte_tpd/` e adicionamos ao Makefile raiz para garantir compilação nativa.
- **Resolução de Violamentos de CFI:** Removemos saltos manuais indiretos de ponteiros de função que disparavam avisos de CFI (Control Flow Integrity), substituindo-os por chamadas formais da API do kernel (como `gpio_free(16)`).
- **Correção do Notifier de Display:** Ajustamos os offsets de leitura do campo `early_trigger` na resposta de notificação do painel de display em `syna_ts_panel_notifier_callback.c`, alinhando a estrutura com as definições GKI.
- **Suporte Multitouch:** Injetamos chamadas a `input_mt_sync_frame` ao registrar toques em `tpd_touch_press.c` e `tpd_touch_release.c`.

---

## 3. Instruções de Compilação e Instalação

1. **Geração dos Objetos e Imagem do Kernel:**
   Execute o script unificado de compilação:
   ```bash
   ./super_build.sh
   ```
2. **Empacotamento com DTB e Assinatura:**
   Gere a imagem de boot com a assinatura de partição necessária para o bootloader userdebug da ZTE:
   ```bash
   ./repack_perfect_sign.sh
   ```
3. **Flashing Temporário para Teste:**
   Conecte o smartphone em modo Fastboot e execute:
   ```bash
   fastboot boot dev_reverse_perfect.img
   ```
