(** Squelette pour le TP26 arbres rouge-noirs en OCaml *)

type 'a rn =
  | V
  | N of 'a rn * 'a * 'a rn
  | R of 'a rn * 'a * 'a rn
;;

(* cons arbre g x d renvoie un nœud :
 * - de la même couleur que la racine de arbre ;
 * - ayant g comme fils gauche, x comme clé et d comme fils droit.contents
 *
 * Noter que la seule chose que l'on utilise dans arbre, c'est la
 * couleur de la racine.
 *
 * Cette fonction ne doit pas être appelée avec arbre = V.
 *
 * Utilisée à bon escient, elle permet d'alléger considérablement
 * le code de insere_aux, supprime_min et supprime_aux en
 * uniformisant le traitement des cas R (g, x, d) et N (g, x, d)
 * (quand ces cas se traitent identiquement). *)
 
 
(* Question 1 *)
let exemple1 = 
  N (
    N (
      V,
      1,
      V
    ),
    2,
    N (
      V,
      3,
      V
    )
  )
;;

let exemple2 = 
  N (
    R (
      N (
        N (
          V,
          10,
          V
        ),
        30,
        N (
          V,
          35,
          V
        )
      ),
      40,
      N (
        N (
          V,
          45,
          V
        ),
        50,
        R (
          N (
            V,
            55,
            V
          ),
          60,
          N (
            V,
            65,
            V
          )
        )
      )
    ),
    70,
    N (
      N (
        V,
        80,
        V
      ),
      90,
      N (
        V,
        100,
        V
      )
    )
  )
;;

(* Question 2 à voir sur la feuille *)

let cons arbre g x d =
  match arbre with
  | N _ -> N (g, x, d)
  | R _ -> R (g, x, d)
  | V -> failwith "erreur cons"
;;

(* La fonction appartient. On peut utiliser
 * des regroupement des cas R _ et N _ :
 * on n'hésitera pas à le faire (quand c'est
 * possible, bien sûr) pour alléger le code. *)
 
let rec appartient t x =
  match t with
  | V -> false
  | N (g, y, d) | R (g, y, d) when x < y -> appartient g x
  | N (g, y, d) | R (g, y, d) when x > y -> appartient d x
  | _ -> true
;;

(* Question 4 et 5 à voir sur la feuille *)

let corrige_rouge t =
  match t with
  | N (R (R (a, x, b), y, c), z, d) 
  | N (R (a, x, R (b, y, c)), z, d)
  | N (a, x, R (R (b, y, c), z, d))
  | N (a, x, R (b, y, R (c, z, d)))
   -> R (N (a, x, b), y, N (c, z, d))
  | _ -> t
;;

let rec insere_aux t x =
  match t with
  | V -> R (V, x, V)
  | N (g, y, d) | R (g, y, d) when x < y -> corrige_rouge (cons t (insere_aux g x) y d)
  | N (g, y, d) | R (g, y, d) when y < x -> corrige_rouge (cons t g y (insere_aux d x))
  | _ -> t (* x est deja dans t *)
;;

let insere t x =
  match insere_aux t x with
  | N (g, y, d) | R (g, y, d) -> N (g, y, d)
  | V -> failwith "insertion echouée"
;;


(**********************************)
(* Fonctions utilitaires fournies *)
(**********************************)

(* La fonction la plus utile est teste_insere ! *)

let construit = List.fold_left insere V
;;

let elements t =
  let rec aux arbre acc =
    match arbre with
    | V -> acc
    | N (g, x, d) | R (g, x, d) ->
      let avec_d = aux d acc in
      aux g (x :: avec_d) in
  aux t []
;;

let rec verifie_rouge = function
  | V -> true
  | R (R _, _, _) | R (_, _, R _) -> false
  | N (l, _, r) | R (l, _, r) -> verifie_rouge l && verifie_rouge r
;;

let rec profondeurs_noires = function
  | V -> 0, 0
  | R (g, _, d) ->
    let ming, maxg = profondeurs_noires g in
    let mind, maxd = profondeurs_noires d in
    (min ming mind, max maxg maxd)
  | N (g, _, d) ->
    let ming, maxg = profondeurs_noires g in
    let mind, maxd = profondeurs_noires d in
    (1 + min ming mind, 1 + max maxg maxd)
;;

let verifie_structure t =
  let mini, maxi = profondeurs_noires t in
  mini = maxi && verifie_rouge t
;;

let verifie_ordre t =
  let rec croissant = function
    | x :: y :: xs -> (x < y) && croissant (y :: xs)
    | _ -> true in
  croissant (elements t)
;;

let verifie_rn t =
  verifie_ordre t && verifie_structure t
;;

let teste_insere () =
  for _ = 1 to 100 do
    let u = List.init 100 (fun _ -> Random.int 100) in
    let t = construit u in
    assert (verifie_rn t);
    assert (elements t = List.sort_uniq compare u)
  done;
  Printf.printf "Insere OK\n"
;;




let repare_noir_gauche arbre a_faire =
  if a_faire then
    match arbre with
    | R (a, x, N (b, y, c)) -> (corrige_rouge (N (corrige_rouge (R (a, x, b)), y, c)), false)
    | N (R(a, x, b), y, c) -> (N (N (a, x, b), y, c), false)
    | N (a, x, N (b, y, c)) -> (corrige_rouge (N (corrige_rouge (R (a, x, b)), y, c)), true)
    | N (a, x, R (N (b, y, c), z, d)) -> (corrige_rouge (N (corrige_rouge (N (a, x, corrige_rouge (R (b, y, c)))), z, d)), false)
    | _ -> failwith "arbre invalide"
  else
    (arbre, false)
;;

repare_noir_gauche (N (N (V, 1, V), 3, R (N (N (V, 4, V), 5, N (V, 6, V)), 7, N (N (V, 8, V), 9, N(V, 10, V))))) true;;

let rec supprime_min arbre =
  match arbre with
  | R (V, x, d) -> (d, false)
  | N (V, x, d) -> (d, true)
  | R (g, x, d) | N (g, x, d)
  -> let g', a_diminue = supprime_min g in
  repare_noir_gauche (cons arbre g' x d) a_diminue
  | V -> failwith "vide"
;;

let repare_noir_droite arbre a_faire =
  if a_faire then
    match arbre with
    | R (N (a, x, b), y, c) -> (corrige_rouge (N (a, x, corrige_rouge (R (b, y, c)))), false)
    | N (a, x, R (b, y, c)) -> (N (a, x, N (b, y, c)), false)
    | N (N (a, x, b), y, c) -> (corrige_rouge (N (a, x, corrige_rouge (R (b, y, c)))), true)
    | N (R (a, x, N (b, y, c)), z, d) -> (corrige_rouge (N (a, x, corrige_rouge (N (R (b, y, c), z, d)))), false)
    | _ -> failwith "arbre invalide"
  else
    (arbre, false)
;;

let rec minimum = function
  | V -> failwith "vide"
  | N (V, x, _) | R (V, x, _) -> x
  | N (g, x, _) | R (g, x, _) -> minimum g
;;

let rec supprime_aux t x =
  match t with
  | V -> (V, false)
  | N (g, y, d) | R (g, y, d) when x < y -> 
    let g', a_diminue = supprime_aux g x in
    repare_noir_gauche (cons t g' y d) a_diminue
  | N (g, y, d) | R (g, y, d) when y < x ->
    let d', a_diminue = supprime_aux d x in
    repare_noir_droite (cons t g y d') a_diminue
  | N (V, y, a) | N (a, y, V)  when x = y -> (a, true)
  | R (V, y, a) | R (a, y, V) when x = y -> (a, false)
  | N (g, y, d) | R (g, y, d) when x = y ->
    let m = minimum d in
    let d', a_diminue = supprime_min d in
    repare_noir_droite (cons t g m d') a_diminue
  | _ -> failwith "arbre invalide"
;;

let supprime t x =
  match supprime_aux t x with
  | V, _ -> V
  | N (g, y, d), _ | R (g, y, d), _ -> N (g, y, d)
;;

let teste_supprime () =
  let rand_int () = Random.int 100 in
  let rand () = (Random.int 2 = 0) in
  let t = ref V in
  let h = Hashtbl.create 50 in
  for _ = 0 to 1_000 do
    let x = rand_int () in
    if rand () then
      begin
        t := supprime !t x;
        Hashtbl.remove h x;
      end
    else
      begin
        t := insere !t x;
        Hashtbl.replace h x ();
      end;
    assert (verifie_rn !t);
    let u = elements !t in
    let v = Hashtbl.to_seq_keys h |> List.of_seq in
    assert (u = List.sort compare v)
  done;
  Printf.printf "Supprime OK\n"
;;
