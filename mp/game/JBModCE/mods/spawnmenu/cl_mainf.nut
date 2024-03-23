function DrawEpicPanel(panel)
{
	if(GetConvar("spawnmenu_enable") == "1")
	{
		VGUISetMouseInputEnabled(panel,true);
		ExecuteConsoleCommand("spawnmenu_enable 0");
	}
	if(GetConvar("spawnmenu_disable") == "1")
	{
		VGUISetMouseInputEnabled(panel,false);
		ExecuteConsoleCommand("spawnmenu_disable 0");
	}
}

paths <- [];


function DrawSpawnButton(button)
{
	SurfaceSetColor(255,255,255,255);
	SurfaceDrawRect(0,0,14,14);
	SurfaceSetColor(0,0,0,255);
	SurfaceDrawRect(1,1,15,15);
	SurfaceSetColor(0,0,255,255);
	SurfaceDrawRect(1,1,14,14);
	SurfaceSetColor(0,0,128,255);
	SurfaceDrawLine(1,13,13,13);
	SurfaceDrawLine(13,13,13,1);
}






function ButtonClicked(button, code)
{
	PrintToClient("dfdsfds: "+VGUIGetUserdata(button)+"\n");
	ExecuteConsoleCommand("SpawnProp " + paths[VGUIGetUserdata(button).tointeger()]);
}


function CreateSpawnButton(parent,x,y,i)
{
	local button = VGUICreatePanel(i.tostring());
	VGUISetParent(button,parent);
	VGUISetBounds(button,x,y,16,16);
	VGUISetPaintFunction(button,"DrawSpawnButton");
	VGUISetOnMouseDownFunction(button,"ButtonClicked");
	VGUISetMouseInputEnabled(button,true);
}

function LevelInitPreEntity()
{
	paths = GetModelList();
	PrintToClient(paths[11]+"\n");
	ExecuteConsoleCommand("setinfo spawnmenu_enable 0");
	ExecuteConsoleCommand("setinfo spawnmenu_disable 0");
	local th = VGUICreatePanel("");
	VGUISetBounds(th,16,16,48,200);
	VGUISetPaintFunction(th,"DrawEpicPanel");
	VGUIMakePopup(th);
	VGUISetMouseInputEnabled(th,false);
	VGUISetKeyBoardInputEnabled(th,false);
	for(local i=0;i<10;i++)
	{
		CreateSpawnButton(th,16,i*16+16,i);
		
	}
}