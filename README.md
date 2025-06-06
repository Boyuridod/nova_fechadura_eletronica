# 🔐 Fechadura Eletrônica com ESP8266 e RFID

Projeto de uma **fechadura eletrônica** utilizando o microcontrolador **ESP8266** que permite o acesso através da leitura de tags **RFID**.

---

## 🚀 Descrição

Este projeto foi desenvolvido para criar uma solução simples e segura de controle de acesso. Quando uma tag RFID autorizada é aproximada do leitor, o ESP8266 libera a trava da fechadura eletrônica, permitindo a abertura.

---

## 🛠️ Componentes Utilizados

- **ESP8266** (NodeMCU ou similar);
- **Módulo RFID RC522**;
- **Ponte H** utilizada como relé para acionar a fechadura;
- Fechadura solenoide;
- Fonte de alimentação adequada;
- Jumpers e protoboard.

---

## 💾 Banco de Dados

- **Firebase** foi usado como banco de dados para gerenciar as tags RFID autorizadas e armazenar registros de acesso.

---

## ⚙️ Funcionamento

1. O ESP8266 lê a tag RFID aproximada do módulo RC522.  
2. Verifica no Firebase se o UID da tag está autorizado.  
3. Caso autorizado, a ponte H é acionada para liberar a trava da porta por alguns segundos.  
4. Caso não autorizado, mantém a trava ativada e pode emitir sinal de erro (LED).

---

## 📋 Código

O código está escrito em C++ para Arduino IDE.

---

## 👨‍💻 Equipe

- Yuri Duarte  
- Vinícius Gabriel  
- Igor Vendramini  
- Maria Fernanda  
- Isabela Iwanow  

**Professor orientador:** Robson Rodrigues

---

## 📚 Contexto do Projeto

Desenvolvido para a disciplina **Projetos Integrados para Engenharia 7° Período** do curso de Engenharia de Computação no IFTM – Campus Uberaba Parque Tecnológico.

---

## 📄 Licença

Projeto desenvolvido para fins acadêmicos e pessoais.  
Não recomendado para uso comercial sem adaptações de segurança.
