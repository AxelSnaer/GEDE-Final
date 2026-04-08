@tool
class_name RMShapeSphere extends RMShape

@export_range(0.0, 90.0, 1.0, "or_greater")
var radius: float = 1.0:
	set(value):
		radius = value
		invalidate_cache()

func _gen_sdf() -> String:
	return "depth = length(pos) - %d;" % [radius]
