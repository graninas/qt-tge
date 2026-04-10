# Qt-TGE: Text-Based Game Editor and Player

## Design Document (Initial Draft)

### Overview
Qt-TGE is an application for authoring and playing text-based games. It provides both an editor for designing game graphs and a player for running game sessions, built on top of a custom engine. The core concept is navigating through a directed, possibly cyclic, graph of locations, with various types of nodes and edges representing the game’s structure.

---

## 1. Game Graph Model

### 1.1. Locations (Nodes)
- **Start Location**
  - Unique per game (only one).
  - Entry point for the player.
  - Must have exactly one outgoing edge.
  - Can have a pack of text descriptions, associated with a Selector (rule for choosing which description to show).
- **Regular Content Locations**
  - Multiple allowed.
  - Represent standard game content or narrative points.
  - Can have a pack of text descriptions, associated with a Selector.
- **Service Locations**
  - Multiple allowed.
  - Used for special logic, services, or non-narrative transitions.
- **Finish Locations**
  - One or more per game.
  - Reaching any finish location ends the game.
  - Can have a pack of text descriptions, associated with a Selector.

### 1.2. Edges
- **Regular Directed Edge**
  - Connects two locations.
  - Represents a possible move or transition.
  - Can have an "Option" text and a "Transition" text.
- **Other Edge Types**
  - To be defined in future iterations (e.g., conditional, random, etc.).

#### Edge Texts and Player Interaction
- When the player is in a location, all "Option" texts from outgoing edges are shown as choices.
- The player selects one option to proceed along the corresponding edge.
- After selection, the player stops "on the chosen edge" and the "Transition" text is displayed.
- There is a single "Continue" option that, when pressed, transfers the player to the destination location.

### 1.3. Graph Properties
- The graph is static during gameplay (no dynamic addition/removal of nodes or edges).
- Cycles are allowed (the player may revisit locations).

---

## 2. Player (Game Session) Model

- **Player**
  - Represents a single game session.
  - Tracks the current location in the graph.
  - Maintains the current game state (dynamic data, variables, inventory, etc.).
  - May include additional session-specific information (to be defined).
- **Game Start**
  - The session begins at the start location.
- **Game End**
  - The session ends when the player reaches any finish location.

---

## 3. Application Components

- **Editor**
  - Visual editor for creating and modifying the game graph.
  - Allows adding, editing, and deleting locations and edges.
  - Supports specifying types for locations and edges.
- **Player**
  - Runs a game session based on the static graph.
  - Provides UI for navigating the graph and interacting with game content.
- **Engine**
  - Core logic for managing the game graph, player state, and transitions.

---

## 4. Future Considerations

- Additional edge types (conditional, service, etc.).
- More detailed game state management.
- Scripting or logic for service locations.
- Save/load functionality for player sessions.
- UI/UX enhancements for both editor and player.

---

## 6. Variables and Game Data

### Location Variables
- Each location may have a number of local named variables.
- Each variable is typed: **string**, **integer**, **float**, **char**, or **bool**.
- Each variable definition must have a default initial value (stored as a string and interpreted according to type).
- While static (in the graph), a variable will have a rule for updating (to be defined later).

### Game Definition
A text game consists of:
- **Name**
- **Locations graph**
- **Global variables**
- **Description**

### Global Variables
- The game may have zero or more global named variables.
- Each variable is typed: **string**, **integer**, **float**, **char**, or **bool**.
- Each variable definition must have a default initial value (stored as a string and interpreted according to type).
- While static (in the game definition), a variable will have a rule for updating (to be defined later).

### Game Dynamic Data
- Current location index
- Current description index
- State of all locations' variables
- State of global variables

---

## Selector
A **Selector** is a mechanism that picks one item from a container (for example, a text descriptions pack) according to a rule:

- **Blank Selector**: Picks the first item in the container.
- **Random Selector**: Picks a random item.
- **Formula Selector**: Uses a script that can refer to the current session's dynamic data as variables and, with the help of math and formulas, computes a number to pick the item. (Script format will be defined later)

A Selector may result in an error. The reaction to such errors will be defined later.

---

## 7. UX Domain Model (Editor-Only Features)

In addition to the core domain model, the application includes a UX domain model to enhance the editing experience. These features are UI-related and do not affect the game logic itself:

- **Location Zones**: Polygon-shaped zones that can be assigned to locations. Each zone can have its own fill color.
- **Location Labels**: Text labels associated with locations, displayed in the editor for clarity.
- **Location Color**: Custom color for each location, used for visual distinction in the editor.
- **Memo Stickers**: Small, text-containing figures that can be placed in the editor for notes or reminders.
- **Location Coordinates**: Each location has unique coordinates (two integer numbers) on an infinite square grid. No two locations may occupy the same coordinates.
