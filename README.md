# Controle da Matrix e Display OLED com botões e Entrada Serial

## Funcionamento

- Botão A: Alterna o estado do led verde
- Botão B: Alterna o estado do led azul
- Entrada Serial:
    - Ao inserir um número: O número é apresentado tanto no display OLED quanto na matriz
    - Ao inserir uma letra: A letra é apresentado no display OLED
- Display OLED: Apresenta 3 informações, última letra ou número digitado pelo usuário, estado do led azul e estado do led verde

# Código

No código foram utilizados interrupções, debouncers via software, uma máquina de estado de um PIO para controle da fita de led que compõe a matriz 5x5, e a aquisição de informações pela entrada serial para operar o sistema.

## Ferramentas Utilizadas no Desenvolvimento

- VS Code
- Linguagem C
- Kit de desenvolvimento de Software Pico (Pico SDK)

## Vídeo de Demonstração

O vídeo de demonstração foi enviado em conjunto com o link para este repositório, na tarefa do dia 27/01 do curso CEPEDI Embarca Tech.

## Execução do projeto

Para executar o projeto é necessário: 
- baixar os arquivos deste repositório
- Utilizando a extensão do Wokwi para VS Code é possível simular o circuito
- Ou, utilizando a extensão Pi Pico para VS Code é possível compilar e carregar o circuito em um microcontrolador Pi Pico W
