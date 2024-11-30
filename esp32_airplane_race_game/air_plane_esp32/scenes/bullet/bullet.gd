extends CharacterBody3D

@export var speed: float = 50.0  # Velocidad de la munición
@export var lifetime: float = 5.0  # Tiempo máximo de vida antes de desaparecer

var direction: Vector3 = Vector3.ZERO  # Dirección de la munición
var timer: float = 0.0  # Contador para el tiempo de vida
var init_speed: float = 0.0

func _ready():
	# Calcular la dirección hacia adelante basada en el eje -Z global
	direction = -global_transform.basis.z.normalized()

func _physics_process(delta: float):
	# Mover la munición estrictamente en la dirección calculada
	translate(direction * (speed + init_speed) * delta)  # Movimiento manual según la dirección

	# Contar el tiempo de vida
	timer += delta
	if timer >= lifetime:
		queue_free()  # Eliminar la munición si supera el tiempo de vida
