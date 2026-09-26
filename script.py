import matplotlib.pyplot as plt
import numpy as np


def graficar_logaritmo(base):
    if base <= 0 or base == 1:
        raise ValueError("La base debe ser mayor que 0 y diferente de 1.")

    # 1. Definir el dominio según el comportamiento de la curva
    # Si la base es decimal pequeña (ej. 0.1), se acerca al infinito en Y muy rápido.
    x = np.linspace(0.01, 10, 500)

    # 2. Aplicar propiedad de cambio de base: log_b(x) = ln(x) / ln(base)
    y = np.log(x) / np.log(base)

    # 3. Configurar la figura
    plt.figure(figsize=(8, 6))

    # 4. Graficar la función principal
    plt.plot(
        x,
        y,
        label=f"$f(x) = \\log_{{{base}}}(x)$",
        color="royalblue",
        linewidth=2.5,
    )

    # 5. Líneas de referencia (Ejes y Asíntota)
    plt.axhline(0, color="black", linestyle="-", linewidth=0.8)  # Eje X
    plt.axvline(
        0,
        color="red",
        linestyle="--",
        linewidth=1.2,
        label="Asíntota vertical (x=0)",
    )

    # 6. Marcar puntos críticos universales
    plt.scatter(1, 0, color="darkorange", zorder=5, s=50)  # Intersección (1,0)
    plt.text(1.2, 0.1, "Intersección (1, 0)", color="darkorange", fontweight="bold")

    # Marcar el punto de la base (base, 1) si entra en el rango visual
    if 0 < base <= 10:
        plt.scatter(base, 1, color="forestgreen", zorder=5, s=50)
        plt.text(
            base + 0.2,
            1,
            f"({base}, 1)",
            color="forestgreen",
            fontweight="bold",
            va="center",
        )

    # 7. Estética de la gráfica
    plt.title(
        f"Gráfica de la Función Logaritmo Base {base}",
        fontsize=14,
        fontweight="bold",
    )
    plt.xlabel("Eje X", fontsize=11)
    plt.ylabel("Eje Y", fontsize=11)
    plt.grid(True, which="both", linestyle=":", alpha=0.5)
    plt.legend(loc="best", fontsize=11)

    # Límites dinámicos para que se vea bien tanto si crece como si decrece
    plt.xlim(-0.5, 11)
    plt.ylim(-3, 3)

    plt.show()


# ==========================================
# 👇 MODIFICA ESTE VALOR PARA CAMBIAR LA BASE 👇
# ==========================================
BASE_DESEADA = 2  # Puedes usar enteros (2, 5) o decimales (0.5, 0.1)

# Llamada a la función
graficar_logaritmo(BASE_DESEADA)