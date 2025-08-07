// scripted_user_func size big
// scripted_user_func size normal
// scripted_user_func size small

function UserConsoleCommand(player, arg)
{
	switch(arg)
	{
		
		case "size big":
			player.SetModelScale(2, 1)
		break
		
		case "size normal":
			player.SetModelScale(1, 1)
		break
		
		case "size small":
			player.SetModelScale(0.5, 1)
		break
		
	}
}