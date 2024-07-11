// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include <common/stats.h>
#include <coins/cores.h>
#include <pools/pools.h>
#include <algo/crypth.h>

#include "ui.h"
#include "ui-dialogs.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//! UiLogin

UiLogin::UiLogin( GuiControlWindow *root ) : GuiDialog("Login")
{
    if( root ) {
        setRoot(*root);
        root->addBinding("login",this);
    }

    setPropertiesWithString(
        "coords={0,0,50%,35%} background=#101010; /body:GuiGroup={ align=top,vertical; coords={0,0,100%,85%}"
            "controls={"
                "info:GuiLabel = { align=top,vertical; coords={40%,0,80%,40%} textcolor=#e01010; text=; textalign=center; }"
                "lblPassword:GuiLabel = { align=top; coords={20%,0,35%,20%} text=Password; textalign=left,centerv; }"
                "password:GuiTextBox = { align=top,vertical; coords={40%,0,80%,20%} text=; }"

                "login:GuiButton = { commandId=2; bind=login; coords={45%,80%,65%,95%} align=none; text=Login; }"
            "}"
        "}"
    );
}

void UiLogin::Open() {
    values()["password"] = "";

    fromString( m_retries ,getMember( values() ,"retry" ,"3" ) );
    m_tries = 0;

    setPropertiesWithString( "/body = { /password={text=123;} }" );
}

bool UiLogin::onConfirm() {
    auto *password = getControlAs_<GuiTextBox>("body/password");
    auto *info = getControlAs_<GuiLabel>("body/info");

    assert( password && info );

//-- verify current password
    const char *s = tocstr(password->text());

    if( !testGlobalPassword(s) ) {
        if( ++m_tries >= m_retries ) exit(-1);

        Format( info->text() ,"Incorrect password (%d/%d)" ,64 ,(int) m_tries ,(int) m_retries );
        return false;
    }

//-- ok
    return globalLogin(s);
}

void UiLogin::onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) {
    switch( commandId ) {
        case GUI_MESSAGEID_OK:
            if( onConfirm() ) Close();
            return;

        default:
            // GuiDialog::onCommand( source ,commandId ,param ,params ,extra );
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////
//! UiPassword

UiPassword::UiPassword( GuiControlWindow *root ) : GuiDialog("Set Password")
    ,m_changePassword(false) ,m_confirmPassword(true)
{
    if( root ) {
        setRoot(*root);
        root->addBinding("dialog",this);
    }

    setPropertiesWithString(
        "coords={0,0,50%,35%} background=#202020; /body:GuiGroup={ align=top,vertical; coords={0,0,100%,85%}"
            "controls={"
                "info:GuiLabel = { align=top,vertical; coords={40%,0,80%,10%} textcolor=#e01010; text=; textalign=center; }"
                "lblCurrent:GuiLabel = { align=top; coords={20%,0,35%,20%} text=Current; textalign=left,centerv; }"
                "current:GuiTextBox = { align=top,vertical; coords={40%,0,80%,20%} text=; }"
                "lblPassword:GuiLabel = { align=top; coords={20%,0,35%,20%} text=Password; textalign=left,centerv; }"
                "password:GuiTextBox = { align=top,vertical; coords={40%,0,80%,20%} text=; }"
                "lblConfirm:GuiLabel = { align=top; coords={20%,0,35%,20%} text=Confirm; textalign=left,centerv; }"
                "confirm:GuiTextBox = { align=top,vertical; coords={40%,0,80%,20%} text=; }"

                "cancel:GuiButton = { commandId=19; bind=dialog; coords={30%,80%,49%,95%} align=none; text=Cancel; } " //! @note command is CLOSE, as intended
                "ok:GuiButton = { commandId=2; bind=dialog; coords={51%,80%,70%,95%} align=none; text=Set; }"
            "}"
        "}"
    );
}

void UiPassword::Open() {

//-- current
    auto *label = getControlAs_<GuiLabel>("body/lblCurrent");
    auto *current = getControlAs_<GuiTextBox>("body/current");

    assert(label && current);

    m_changePassword = ( *getMember( values() ,"current" ,"" ) != 0 );

    label->visible() = m_changePassword;
    current->visible() = m_changePassword;
    current->coords().bottom = m_changePassword ? 20.f : 5.f;

//-- confirm
    auto *lblConfirm = getControlAs_<GuiLabel>("body/lblConfirm");
    auto *confirm = getControlAs_<GuiTextBox>("body/confirm");

    assert(lblConfirm && confirm);

    fromString( m_confirmPassword ,getMember( values() ,"confirm" ,"true" ) );

    lblConfirm->visible() = m_confirmPassword;
    confirm->visible() = m_confirmPassword;

//-- out
    values()["password"] = "";
    values()["set"] = "false";

    setPropertiesWithString( "/body = { /current={text=;} /password={text=;} /confirm={text=;} }" );
}

bool UiPassword::onConfirm() {
    auto *current = getControlAs_<GuiTextBox>("body/current");
    auto *password = getControlAs_<GuiTextBox>("body/password");
    auto *confirm = getControlAs_<GuiTextBox>("body/confirm");
    auto *info = getControlAs_<GuiLabel>("body/info");

    assert( current && password && confirm && info );

//-- verify current password
    if( m_changePassword ) {
        const char *s = getMember( values() ,"current" ,"" );

        String r;
        DecryptH( tocstr(current->text()) ,s ,r );

        if( current->text() != r ) {
            info->text() = "Incorrect current password";
            return false;
        }
    }

//-- confirm
    if( m_confirmPassword && password->text() != confirm->text() ) {
        info->text() = "Passwords are different";
        return false;
    }

//-- empty
    bool allowEmpty = false;

    if( !fromString( allowEmpty ,getMember( values() ,"allow-empty" ,"false" ) ) && password->text().empty() ) {
        info->text() = "Empty passwords not allowed";
        return false;
    }

//-- ok, out values
    values()["password"] = password->text();
    values()["set"] = "true";

    return true;
}

void UiPassword::onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) {
    switch( commandId ) {
        case GUI_MESSAGEID_OK:
            if( onConfirm() ) Close();
            return;

        default:
            GuiDialog::onCommand( source ,commandId ,param ,params ,extra );
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF
