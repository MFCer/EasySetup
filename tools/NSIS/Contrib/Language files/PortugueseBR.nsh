﻿;Language: Brazilian Portuguese (1046)
;By Felipe

;Modificado em 21/09/2009 pela equipe MEPS
;     - Mudança do $(^NameDA) para gênero feminino.
;     - Mudou valor de MUI_TEXT_FINISH_INFO_TITLE para caber na caixa
;     - MUI_UNTEXT_FINISH_INFO_TITLE - mudança de completando para concluindo
;     - MUI_TEXT_FINISH_SHOWREADME - mudança de Readme para Leiame

!insertmacro LANGFILE "PortugueseBR" "Brazilian Portuguese" "Português Brasileiro" "Portugues Brasileiro"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Bem-vindo ao Assistente de Instalação da $(^NameDA)"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Este assistente guiará você através da instalação da $(^NameDA).$\r$\n$\r$\nÉ recomendado que você feche todos os outros aplicativos antes de iniciar o instalador. Isto tornará possível atualizar os arquivos de sistema relevantes sem ter que reiniciar seu computador.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Bem-vindo ao Assistente de Desinstalação da $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Este assistente guiará você através da desinstalação da $(^NameDA).$\r$\n$\r$\nAntes de iniciar a desinstalação, tenha certeza de que a $(^NameDA) não está em execução.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Acordo da licença"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Por favor, reveja os termos da licença antes de instalar a $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Se você aceita os termos do acordo, clique em Eu Concordo para continuar. Você deve aceitar o acordo para instalar a $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Se você aceita os termos do acordo, clique na caixa de seleção abaixo. Você deve aceitar o acordo para instalar a $(^NameDA). $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Se você aceita os termos do acordo, selecione a primeira opção abaixo. Você deve aceitar o acordo para instalar a $(^NameDA). $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Acordo da licença"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Por favor, reveja os termos da licença antes de desinstalar a $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Se você aceita os termos do acordo, clique em Eu Concordo para continuar. Você deve aceitar o acordo para desinstalar a $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Se você aceita os termos do acordo, clique na caixa de seleção abaixo. Você deve aceitar o acordo para desinstalar a $(^NameDA). $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Se você aceita os termos do acordo, selecione a primeira opção abaixo. Você deve aceitar o acordo para desinstalar a $(^NameDA). $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Pressione Page Down para ver o resto do acordo."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Escolher Componentes"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Escolha quais funções da $(^NameDA) você quer instalar."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Escolher Componentes"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Escolha quais funções da $(^NameDA) você quer desinstalar."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Descrição"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Posicione seu mouse sobre um componente para ver sua descrição."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Selecione um componente para ver sua descrição."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Escolher o Local da Instalação"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Escolha a pasta na qual instalar a $(^NameDA)."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Escolher o Local da Desinstalação"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Escolha a pasta da qual desinstalar a $(^NameDA)."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Instalando"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Por favor espere enquanto a $(^NameDA) está sendo instalada."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Instalação Completa"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "O instalador completou com sucesso."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Instalação Abortada"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "O instalador não completou com sucesso."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Desinstalando"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Por favor espere enquanto a $(^NameDA) está sendo desinstalada."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Desinstalação Completa"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "A desinstalação foi completada com sucesso."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Desinstalação Abortada"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "A desinstalação não foi completada com sucesso."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Concluindo a instalação da $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "A $(^NameDA) foi instalada no seu computador.$\r$\n$\r$\nClique em Terminar para fechar este assistente."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Seu computador deve ser reiniciado para completar a instalação da $(^NameDA). Você quer reiniciar agora?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Concluindo a desinstalação da $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) foi desinstalado do seu computador.$\r$\n$\r$\nClique em Terminar para fechar este assistente."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Seu computador tem que ser reiniciado para completar a desinstalação da $(^NameDA). Você quer reiniciar agora?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Reiniciar agora"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Eu quero reiniciar manualmente depois"
  ${LangFileString} MUI_TEXT_FINISH_RUN "&Executar $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "&Mostrar o Leiame"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Terminar"
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Escolher a Pasta do Menu Iniciar"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Escolher uma pasta do Menu Iniciar para os atalhos da $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Selecione a pasta do Menu Iniciar na qual você gostaria de criar os atalhos do programa. Você pode também inserir um nome para criar uma nova pasta."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Não criar atalhos"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Desinstalar a $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Remover a $(^NameDA) do seu computador."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Você tem certeza de que quer sair do instalador da $(^Name)?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Você tem certeza de que quer sair da Desinstalação da $(^Name)?"
!endif

!ifdef MULTIUSER_INSTALLMODEPAGE
  ${LangFileString} MULTIUSER_TEXT_INSTALLMODE_TITLE "Escolher Usuários"
  ${LangFileString} MULTIUSER_TEXT_INSTALLMODE_SUBTITLE "Escolher para quais usuários você quer instalar a $(^NameDA)."
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_TOP "Selecione se você quer instalar a $(^NameDA) para si mesmo ou para todos os usuários deste computador. $(^ClickNext)"
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_ALLUSERS "Instalar para qualquer um usando este computador"
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_CURRENTUSER "Instalar apenas para mim"
!endif
