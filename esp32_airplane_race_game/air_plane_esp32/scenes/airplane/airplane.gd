extends CharacterBody3D


# Nodo de la munición
@onready var BULLET = preload("res://scenes/bullet/bullet.tscn")

# Can't fly below this speed
var min_flight_speed = 5
# Maximum airspeed
var max_flight_speed = 40
# Turn rate
var turn_speed = 0.75
# Climb/dive rate
var pitch_speed = 1.5
# Wings "autolevel" speed
var level_speed = 3.0
# Throttle change speed
var throttle_delta = 50
# Acceleration/deceleration
var acceleration = 6.0

# Current speed
var forward_speed = 40
# Throttle input speed
var target_speed = 0
# Lets us change behavior when grounded
var grounded = false

var turn_input = 0
var pitch_input = 0

var can_shoot = true

@onready var mesh = $mesh

func get_input(delta):
	# Throttle input
	if Input.is_action_pressed("throttle_up"):
		target_speed = min(forward_speed + throttle_delta * delta, max_flight_speed)
	if Input.is_action_pressed("throttle_down"):
		target_speed = max(forward_speed - throttle_delta * delta, max_flight_speed)

	turn_input = -Input.get_axis("ui_right", "ui_left")

	pitch_input =  -Input.get_axis("ui_down", "ui_up")
	
	# Manejar disparo con el botón 1 del joystick
	if Input.is_action_just_pressed("shoot"):
		spawn_bullet()

func _physics_process(delta):
	get_input(delta)
	transform.basis = transform.basis.rotated(transform.basis.x, pitch_input * pitch_speed * delta)
	transform.basis = transform.basis.rotated(Vector3.UP, turn_input * turn_speed * delta)
	
	# Bank when turning
	if grounded:
		mesh.rotation.z = 0
	else:
		mesh.rotation.z = lerpf(mesh.rotation.z, -turn_input, level_speed * delta)

	# Accelerate/decelerate
	forward_speed = lerpf(forward_speed, target_speed, acceleration * delta)

	# Movement is always forward
	velocity = -transform.basis.z * forward_speed

	move_and_slide()


func spawn_bullet():
	# Crear una nueva munición en la dirección del Muzzle
	if BULLET:
		var bullet = BULLET.instantiate()
		bullet.init_speed = forward_speed
		get_parent().add_child(bullet)  # Agregar al nivel o nodo raíz
		bullet.global_transform = $Muzzle.global_transform  # Alinear posición y rotación con el Muzzle
