const DIGITS = "0123456789abcdefghijklmnopqrstuvwxyz";

function parseToBigInt(value, base) {
  const normalized = value.trim().toLowerCase();
  if (!normalized) {
    throw new Error("请输入要转换的数值。");
  }

  const sign = normalized.startsWith("-") ? -1n : 1n;
  const unsigned = normalized.replace(/^[+-]/, "");
  if (!unsigned) {
    throw new Error("请输入有效整数。");
  }

  let decimalValue = 0n;
  const bigBase = BigInt(base);

  for (const char of unsigned) {
    const digit = DIGITS.indexOf(char);
    if (digit < 0 || digit >= base) {
      throw new Error(`字符 “${char}” 不适用于 ${base} 进制。`);
    }
    decimalValue = decimalValue * bigBase + BigInt(digit);
  }

  return decimalValue * sign;
}

function formatFromBigInt(value, base) {
  if (base < 2 || base > 36) {
    throw new Error("进制必须在 2~36 之间。");
  }
  return value.toString(base).toUpperCase();
}

function readBase(inputValue, label) {
  const base = Number(inputValue);
  if (!Number.isInteger(base) || base < 2 || base > 36) {
    throw new Error(`${label}必须是 2~36 之间的整数。`);
  }
  return base;
}

const form = document.querySelector("#converter-form");
const sourceInput = document.querySelector("#source-value");
const fromBaseInput = document.querySelector("#from-base");
const toBaseInput = document.querySelector("#to-base");
const resultValue = document.querySelector("#result-value");
const resultError = document.querySelector("#result-error");

form.addEventListener("submit", (event) => {
  event.preventDefault();

  try {
    const fromBase = readBase(fromBaseInput.value, "原始进制");
    const toBase = readBase(toBaseInput.value, "目标进制");

    const decimalValue = parseToBigInt(sourceInput.value, fromBase);
    resultValue.textContent = formatFromBigInt(decimalValue, toBase);
    resultError.textContent = "";
  } catch (error) {
    resultValue.textContent = "-";
    resultError.textContent = error.message;
  }
});
