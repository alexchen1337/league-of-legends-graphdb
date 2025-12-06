import random
import argparse


def generate_html(matches: int, seed: int) -> str:
    rng = random.Random(seed)
    lines = []
    for i in range(matches):
        game_id = f"game-{i}"
        lines.append(f'<div class="game" data-game-id="{game_id}">')
        for slot in range(10):
            summoner = f"player-{i}-{slot}"
            champ_key = rng.randint(1, 200)
            win = 1 if slot < 5 else 0
            lines.append(
                f'<div class="participant" data-summoner-id="{summoner}" '
                f'data-champion-key="{champ_key}" data-team-win="{win}"></div>'
            )
        lines.append("</div>")
    return "\n".join(lines)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--matches", type=int, default=2)
    ap.add_argument("--seed", type=int, default=7)
    ap.add_argument("--out", type=str, default="synthetic_input.html")
    args = ap.parse_args()

    html = generate_html(args.matches, args.seed)
    with open(args.out, "w", encoding="utf-8") as f:
        f.write(html)
    print(f"wrote {args.out}")


if __name__ == "__main__":
    main()

