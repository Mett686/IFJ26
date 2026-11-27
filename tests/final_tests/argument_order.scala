import ifj26.*

def printValues(first: Int, second: Int, third: Int): Unit =
  print(first)
  print(",")
  print(second)
  print(",")
  print(third)
  print("\n")

@main def argumentOrderExample(): Unit =
  val first = 11
  val second = 22
  val third = 33
  printValues(first, second, third)
