BINDING_HEADER_INTERACT = 'Interact'

StaticPopupDialogs['INTERACT_WARNING'] = {
   text = 'Failed to load |cffffd200Interact|cffffffff. Follow the installation instructions to ensure proper installation.',
   button1 = 'Okay',
   timeout = 0,
   whileDead = true,
}

if not InteractNearest then
   StaticPopup_Show('INTERACT_WARNING')
end

function Interact(autoloot)
    if not InteractNearest then
        StaticPopup_Show('INTERACT_WARNING')
        return
    end

    InteractNearest(autoloot)
end

-- /interact in a macro. "/interact loot" auto-loots, like the second binding.
SLASH_INTERACT1 = '/interact'
SlashCmdList['INTERACT'] = function(msg)
    if string.lower(msg or '') == 'loot' then
        Interact(1)
    else
        Interact(0)
    end
end
