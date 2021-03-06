/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 312230917
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif/*__cplusplus*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"

#include "api_ui.inc"
#include "api_ui.h"
#include "api_ui_help.h"
#include "api_ui_printf.h"


/*---------------------------------------------------------------------------
 * Subroutine Name: api_ui_help_init
 *
 * Input                Description
 * -----                -----------
 * - none -
 *
 * Output               Description
 * ------               -----------
 * - none -
 *
 * Description: This function initializes the UI structures and installs
 *              the API_UI_HELP UI commands.
 *
 *              THIS FUNCTION IS GENERATED BY UIParser, DO NOT EDIT!!!
 *---------------------------------------------------------------------------*/
void api_ui_help_init(void)
{
    /* DECLARATIONS */

    /* ELEMENT DEFINITIONS */

    /* COMMAND DEFINITIONS */
    api_ui_secure_define(API_UI_SECURITY_USER, ui_display_help,
                   "help",
                   "help");
}

/*---------------------------------------------------------------------------
 * Subroutine Name: ui_display_help
 *
 * Input        Description
 * -----        -----------
 * param       required API_UI_PARAM for all ui commands, not used
 *
 * Output       Description
 * ------       -----------
 * char*       unused, required for ui command functions
 *
 * Description:
 *     Prints out available top level ui commands
 *
 *---------------------------------------------------------------------------*/
char* ui_display_help(API_UI_PARAM *param)
{
    API_UI_NODE *ui_node_ptr;

    api_ui_printf("%16s\t\t%s\n", "command abbr.", "command syntax");
    api_ui_printf("---------------------------------------------------------------------------------------------------\n");

    ui_node_ptr = api_ui_cmd_tree();
    while(ui_node_ptr != NULL)
    {
        api_ui_printf("%16s\t\t%s\n", ui_node_ptr->element->word, ui_node_ptr->element->help);
        ui_node_ptr = ui_node_ptr->right;
    }
    api_ui_printf("\n");   /* additional spare line after help */
    return(NULL);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

