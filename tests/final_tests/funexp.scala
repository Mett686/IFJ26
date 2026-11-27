// Rozsireni FUNEXP
import ifj26.*

def traced(value: Int): Int =
  print("[")
  print(value)
  print("]")
  value

@main def funexpExample(): Unit =
  val result = traced(2) + 3 * traced(traced(4) + 1)  // vyrazy jako argumenty, volani uvnitr vyrazu
  // na vystupu si povsimnete, v jakem poradi jsou funkce volany!
  print("\nresult: ")
  print(toStr(result))
  print("\n")
