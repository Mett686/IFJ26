import ifj26.*

@main def numericOpsExample(): Unit =
  val precedence = 7 + 3 * 2
  val truncated = 0 - 7 / 3
  val mixed: Double = precedence + 0.5
  val mixedText = toStr(mixed)
  print("precedence: ")
  print(precedence)
  print("\ntruncated: ")
  print(truncated)
  print("\nmixed: ")
  print(mixedText)
  print("\n")
  if precedence > truncated then
    print("numbers: ordered\n")
  else
    print("numbers: unexpected\n")
